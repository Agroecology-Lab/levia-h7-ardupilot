/*
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 */

#include <utility>

#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>

#include "AP_InertialSensor_HXY42688.h"

extern const AP_HAL::HAL &hal;

static constexpr uint8_t HXY_REG_BANK_SEL = 0x7f;
static constexpr uint8_t HXY_REG_WHO_AM_I = 0x01;
static constexpr uint8_t HXY_WHO_AM_I = 0x6a;
static constexpr uint8_t HXY_REG_RAW_DATA = 0x0c;
static constexpr uint16_t HXY_SAMPLE_RATE_HZ = 100;
// Match the configured 100 Hz output data rate. Polling at 200 Hz can submit
// the same latched sample twice if DRDY remains asserted until the next update.
static constexpr uint32_t HXY_POLL_PERIOD_US = 10000;

// ACC_RANGE=1 selects +/-4 g; GYR_RANGE=0 selects +/-2000 degrees/second.
static constexpr float HXY_ACCEL_SCALE = (4.0f * GRAVITY_MSS) / 32768.0f;
static constexpr float HXY_GYRO_SCALE = radians(2000.0f) / 32768.0f;

AP_InertialSensor_HXY42688::AP_InertialSensor_HXY42688(
    AP_InertialSensor &imu,
    AP_HAL::OwnPtr<AP_HAL::Device> dev,
    enum Rotation rotation_in)
    : AP_InertialSensor_Backend(imu)
    , _dev(std::move(dev))
    , _rotation(rotation_in)
{
}

AP_InertialSensor_Backend *AP_InertialSensor_HXY42688::probe(
    AP_InertialSensor &imu,
    AP_HAL::OwnPtr<AP_HAL::SPIDevice> dev,
    enum Rotation rotation_in)
{
    if (!dev) {
        return nullptr;
    }

    auto *sensor = NEW_NOTHROW AP_InertialSensor_HXY42688(imu, std::move(dev), rotation_in);
    if (sensor == nullptr || !sensor->hardware_init()) {
        delete sensor;
        return nullptr;
    }
    return sensor;
}

bool AP_InertialSensor_HXY42688::hardware_init()
{
    WITH_SEMAPHORE(_dev->get_semaphore());

    _dev->set_speed(AP_HAL::Device::SPEED_LOW);
    _dev->set_read_flag(0x80);

    if (!_dev->write_register(HXY_REG_BANK_SEL, 0x00, false)) {
        return false;
    }

    uint8_t id = 0;
    if (!_dev->read_registers(HXY_REG_WHO_AM_I, &id, 1) || id != HXY_WHO_AM_I) {
        return false;
    }

    struct RegisterValue {
        uint8_t reg;
        uint8_t value;
        uint16_t delay_ms;
    };
    static constexpr RegisterValue setup[] = {
        { HXY_REG_BANK_SEL, 0x00, 1 },
        { 0x7d, 0x0e, 200 }, // power accel, gyro and temperature
        { 0x40, 0x88, 2 },   // accelerometer 100 Hz
        { 0x41, 0x01, 2 },   // accelerometer +/-4 g
        { 0x42, 0xc8, 2 },   // gyro 100 Hz
        { 0x43, 0x00, 2 },   // gyro +/-2000 degrees/second
        { 0x05, 0x50, 2 },   // enable auto-increment for the raw data block
        { 0x08, 0x05, 2 },   // digital low-pass filter
        { 0x1c, 0x00, 1 },   // keep FIFO disabled; use raw registers
        { 0x1d, 0x00, 1 },   // FIFO bypass mode
    };

    for (const auto &item : setup) {
        if (!_dev->write_register(item.reg, item.value, false)) {
            return false;
        }
        hal.scheduler->delay(item.delay_ms);
    }
    return true;
}

void AP_InertialSensor_HXY42688::start()
{
    const uint32_t device_id = _dev->get_bus_id_devtype(DEVTYPE_INS_HXY42688);
    if (!_imu.register_accel(accel_instance, HXY_SAMPLE_RATE_HZ, device_id) ||
        !_imu.register_gyro(gyro_instance, HXY_SAMPLE_RATE_HZ, device_id)) {
        return;
    }

    set_accel_orientation(accel_instance, _rotation);
    set_gyro_orientation(gyro_instance, _rotation);

    periodic_handle = _dev->register_periodic_callback(
        HXY_POLL_PERIOD_US,
        FUNCTOR_BIND_MEMBER(&AP_InertialSensor_HXY42688::read_sensor, void));
}

static int16_t read_be16(const uint8_t *data)
{
    return int16_t((uint16_t(data[0]) << 8) | data[1]);
}

void AP_InertialSensor_HXY42688::read_sensor()
{
    // The clone's data-ready bits are not reliably asserted. Poll the raw
    // block at the configured 100 Hz rate, as recommended by the proven
    // non-FIFO vendor path, so ArduPilot never sees multi-second data gaps.
    uint8_t raw[12];
    if (!_dev->read_registers(HXY_REG_RAW_DATA, raw, sizeof(raw))) {
        return;
    }

    Vector3f accel(read_be16(&raw[0]), read_be16(&raw[2]), read_be16(&raw[4]));
    Vector3f gyro(read_be16(&raw[6]), read_be16(&raw[8]), read_be16(&raw[10]));
    accel *= HXY_ACCEL_SCALE;
    gyro *= HXY_GYRO_SCALE;

    _rotate_and_correct_accel(accel_instance, accel);
    _rotate_and_correct_gyro(gyro_instance, gyro);
    _notify_new_accel_raw_sample(accel_instance, accel);
    _notify_new_gyro_raw_sample(gyro_instance, gyro);
}

bool AP_InertialSensor_HXY42688::update()
{
    update_accel(accel_instance);
    update_gyro(gyro_instance);
    return true;
}
