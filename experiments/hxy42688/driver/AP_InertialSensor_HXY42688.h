#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/SPIDevice.h>

#include "AP_InertialSensor.h"
#include "AP_InertialSensor_Backend.h"

/*
 * Driver for the HXY/Silan ICM-42688P pin-compatible device.
 *
 * This part is not register-compatible with the TDK ICM-42688P.  It uses a
 * bank selector at 0x7f, reports 0x6a at bank-0 register 0x01, and exposes a
 * 16-bit accel/gyro data block beginning at 0x0c.
 */
class AP_InertialSensor_HXY42688 : public AP_InertialSensor_Backend
{
public:
    static AP_InertialSensor_Backend *probe(AP_InertialSensor &imu,
                                            AP_HAL::OwnPtr<AP_HAL::SPIDevice> dev,
                                            enum Rotation rotation_in);

    void start() override;
    bool update() override;

private:
    AP_InertialSensor_HXY42688(AP_InertialSensor &imu,
                               AP_HAL::OwnPtr<AP_HAL::Device> dev,
                               enum Rotation rotation_in);

    bool hardware_init();
    void read_sensor();

    AP_HAL::OwnPtr<AP_HAL::Device> _dev;
    AP_HAL::Device::PeriodicHandle periodic_handle;
    const enum Rotation _rotation;
};
