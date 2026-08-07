#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/SPIDevice.h>

const AP_HAL::HAL &hal = AP_HAL::get_HAL();

static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu1;
static bool imu1_ready;
static constexpr uint8_t LED0_PIN = 90;
static constexpr uint8_t LED1_PIN = 91;

static void set_leds(bool led0, bool led1)
{
    // Both board LEDs are wired active low.
    hal.gpio->write(LED0_PIN, !led0);
    hal.gpio->write(LED1_PIN, !led1);
}

static void configure_device(AP_HAL::SPIDevice *dev)
{
    if (dev == nullptr) {
        return;
    }
    dev->set_read_flag(0x80);
    dev->set_speed(AP_HAL::Device::SPEED_LOW);
}

static bool write_imu1(uint8_t reg, uint8_t value)
{
    WITH_SEMAPHORE(imu1->get_semaphore());
    return imu1->write_register(reg, value, false);
}

static bool initialise_imu1()
{
    uint8_t id = 0;
    {
        WITH_SEMAPHORE(imu1->get_semaphore());
        if (!imu1->write_register(0x7F, 0x00, false) ||
            !imu1->read_registers(0x01, &id, 1)) {
            return false;
        }
    }
    if (id != 0x6A) {
        return false;
    }

    // Minimal non-FIFO setup taken from the Silan/HXY vendor driver.
    const struct {
        uint8_t reg;
        uint8_t value;
    } setup_registers[] = {
        { 0x7F, 0x00 }, // bank 0
        { 0x7D, 0x0E }, // accelerometer + gyro + temperature on
        { 0x40, 0x88 }, // accelerometer, 100 Hz
        { 0x41, 0x01 }, // accelerometer range: +/-4 g
        { 0x42, 0xC8 }, // gyroscope, 100 Hz
        { 0x43, 0x00 }, // gyroscope range: +/-2000 dps
        { 0x05, 0x50 }, // communication configuration
        { 0x08, 0x05 }, // enable DLPF
    };

    for (const auto &item : setup_registers) {
        if (!write_imu1(item.reg, item.value)) {
            return false;
        }
        hal.scheduler->delay(item.reg == 0x7D ? 200 : 2);
    }
    return true;
}

void setup()
{
    hal.console->printf("LEVIA_H7 HXY/Silan visual tilt test\n");
    hal.console->printf("level=both; X=one LED; +Y=both blink; -Y=alternate; failure=fast alternate\n");

    hal.gpio->pinMode(LED0_PIN, HAL_GPIO_OUTPUT);
    hal.gpio->pinMode(LED1_PIN, HAL_GPIO_OUTPUT);
    set_leds(false, false);

    imu1 = hal.spi->get_device("icm42688_1");
    configure_device(imu1.get());

    if (imu1) {
        imu1_ready = initialise_imu1();
    }
    hal.console->printf("SPI1 sensor initialisation: %s\n",
                        imu1_ready ? "OK" : "FAIL");
}

void loop()
{
    static bool phase;
    phase = !phase;

    if (!imu1_ready) {
        set_leds(phase, !phase);
        hal.scheduler->delay(125);
        return;
    }

    uint8_t status = 0;
    uint8_t raw[12]{};
    bool read_ok;
    {
        WITH_SEMAPHORE(imu1->get_semaphore());
        read_ok = imu1->read_registers(0x0B, &status, 1) &&
                  imu1->read_registers(0x0C, raw, sizeof(raw));
    }

    const auto be16 = [](const uint8_t *p) -> int16_t {
        return int16_t((uint16_t(p[0]) << 8) | p[1]);
    };
    const int16_t ax = be16(&raw[0]);
    const int16_t ay = be16(&raw[2]);
    const int16_t az = be16(&raw[4]);

    if (!read_ok || (status & 0x03) != 0x03) {
        set_leds(phase, !phase);
        hal.scheduler->delay(125);
        return;
    }

    // At the configured +/-4 g range, 2000 counts is a clearly visible tilt.
    static constexpr int16_t tilt_threshold = 2000;
    const int32_t abs_x = labs(int32_t(ax));
    const int32_t abs_y = labs(int32_t(ay));

    if (abs_x < tilt_threshold && abs_y < tilt_threshold) {
        set_leds(true, true);             // approximately level
    } else if (abs_x >= abs_y) {
        set_leds(ax < 0, ax >= 0);        // X direction chooses one LED
    } else if (ay >= 0) {
        set_leds(phase, phase);           // +Y: blink together
    } else {
        set_leds(phase, !phase);          // -Y: blink alternately
    }

    hal.console->printf("A=[%d,%d,%d]\n", int(ax), int(ay), int(az));
    hal.scheduler->delay(150);
}

AP_HAL_MAIN();
