#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/SPIDevice.h>

const AP_HAL::HAL &hal = AP_HAL::get_HAL();

static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu1;
static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu2;
static bool sensors_ready;
static uint8_t led0_level;
static uint8_t led1_level;
static uint8_t led0_target;
static uint8_t led1_target;
static uint8_t consecutive_read_failures;

static constexpr uint8_t LED0_PIN = 90;
static constexpr uint8_t LED1_PIN = 91;

struct AccelSample {
    int16_t x;
    int16_t y;
    int16_t z;
};

static void set_leds(bool led0, bool led1)
{
    // LEVIA_H7 LEDs are active low.
    hal.gpio->write(LED0_PIN, !led0);
    hal.gpio->write(LED1_PIN, !led1);
}

static void configure_device(AP_HAL::SPIDevice *dev)
{
    if (dev != nullptr) {
        dev->set_read_flag(0x80);
        dev->set_speed(AP_HAL::Device::SPEED_LOW);
    }
}

static bool write_register(AP_HAL::SPIDevice *dev, uint8_t reg, uint8_t value)
{
    WITH_SEMAPHORE(dev->get_semaphore());
    return dev->write_register(reg, value, false);
}

static bool initialise_device(AP_HAL::SPIDevice *dev)
{
    if (dev == nullptr) {
        return false;
    }

    uint8_t id = 0;
    {
        WITH_SEMAPHORE(dev->get_semaphore());
        if (!dev->write_register(0x7F, 0x00, false) ||
            !dev->read_registers(0x01, &id, 1)) {
            return false;
        }
    }
    if (id != 0x6A) {
        return false;
    }

    const struct {
        uint8_t reg;
        uint8_t value;
    } setup_registers[] = {
        { 0x7F, 0x00 }, { 0x7D, 0x0E },
        { 0x40, 0x88 }, { 0x41, 0x01 },
        { 0x42, 0xC8 }, { 0x43, 0x00 },
        { 0x05, 0x50 }, { 0x08, 0x05 },
    };

    for (const auto &item : setup_registers) {
        if (!write_register(dev, item.reg, item.value)) {
            return false;
        }
        hal.scheduler->delay(item.reg == 0x7D ? 200 : 2);
    }
    return true;
}

static int16_t be16(const uint8_t *p)
{
    return int16_t((uint16_t(p[0]) << 8) | p[1]);
}

static bool read_accel(AP_HAL::SPIDevice *dev, AccelSample &sample)
{
    uint8_t status = 0;
    uint8_t raw[12]{};
    bool ok;
    {
        WITH_SEMAPHORE(dev->get_semaphore());
        ok = dev->read_registers(0x0B, &status, 1) &&
             dev->read_registers(0x0C, raw, sizeof(raw));
    }

    // DRDY may momentarily be clear when polling between 100 Hz samples. The
    // data registers still hold the latest complete sample, so only an SPI
    // transfer failure is fatal here.
    if (!ok) {
        return false;
    }
    sample.x = be16(&raw[0]);
    sample.y = be16(&raw[2]);
    sample.z = be16(&raw[4]);
    return true;
}

static uint8_t brightness_from_axis(int32_t value)
{
    // 50% brightness when level. About +/-4000 counts spans the full range.
    int32_t brightness = 50 + value / 80;
    if (brightness < 0) {
        brightness = 0;
    } else if (brightness > 100) {
        brightness = 100;
    }
    return uint8_t(brightness);
}

static void update_fused_tilt()
{
    AccelSample a1{};
    AccelSample a2{};
    if (!read_accel(imu1.get(), a1) || !read_accel(imu2.get(), a2)) {
        if (++consecutive_read_failures >= 20) {
            sensors_ready = false;
        }
        return;
    }
    consecutive_read_failures = 0;

    // PCB placement rotates IMU2 90 degrees relative to IMU1.
    // Convert IMU2 to the IMU1 frame: X=-Y2, Y=X2, Z=Z2.
    const int32_t fused_x = (int32_t(a1.x) - int32_t(a2.y)) / 2;
    const int32_t fused_y = (int32_t(a1.y) + int32_t(a2.x)) / 2;

    led0_target = brightness_from_axis(fused_x);
    led1_target = brightness_from_axis(fused_y);

    // Slew by one percent per sample to make the colour blend visibly smooth.
    if (led0_level < led0_target) {
        led0_level++;
    } else if (led0_level > led0_target) {
        led0_level--;
    }
    if (led1_level < led1_target) {
        led1_level++;
    } else if (led1_level > led1_target) {
        led1_level--;
    }

    static uint8_t report_count;
    if (++report_count >= 25) {
        report_count = 0;
        hal.console->printf("fused=[%ld,%ld] pwm=[%u,%u] target=[%u,%u]\n",
                            long(fused_x), long(fused_y),
                            unsigned(led0_level), unsigned(led1_level),
                            unsigned(led0_target), unsigned(led1_target));
    }
}

void setup()
{
    hal.console->printf("LEVIA_H7 dual-HXY smooth LED fusion test\n");
    hal.gpio->pinMode(LED0_PIN, HAL_GPIO_OUTPUT);
    hal.gpio->pinMode(LED1_PIN, HAL_GPIO_OUTPUT);
    set_leds(false, false);

    imu1 = hal.spi->get_device("icm42688_1");
    imu2 = hal.spi->get_device("icm42688_2");
    configure_device(imu1.get());
    configure_device(imu2.get());
    sensors_ready = initialise_device(imu1.get()) && initialise_device(imu2.get());
    led0_level = led1_level = 50;
    led0_target = led1_target = 50;
    hal.console->printf("Dual sensor initialisation: %s\n",
                        sensors_ready ? "OK" : "FAIL");
}

void loop()
{
    static uint8_t pwm_phase;
    static uint32_t last_sample_ms;

    const uint32_t now = AP_HAL::millis();
    if (sensors_ready && now - last_sample_ms >= 20) {
        last_sample_ms = now;
        update_fused_tilt();
    }

    if (!sensors_ready) {
        // Obvious error indication if either sensor fails.
        if ((now / 125U) & 1U) {
            set_leds(true, false);
        } else {
            set_leds(false, true);
        }
        hal.scheduler->delay_microseconds(500);
        return;
    }

    set_leds(pwm_phase < led0_level, pwm_phase < led1_level);
    pwm_phase = uint8_t((pwm_phase + 1U) % 100U);
    hal.scheduler->delay_microseconds(50);
}

AP_HAL_MAIN();
