#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/SPIDevice.h>

const AP_HAL::HAL &hal = AP_HAL::get_HAL();

static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu1;
static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu2;

static constexpr uint8_t LED0_PIN = 90;
static constexpr uint8_t LED1_PIN = 91;
static constexpr uint8_t WHO_AM_I_REG = 0x01;
static constexpr uint8_t EXPECTED_ID = 0x6A;

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

static bool read_whoami(AP_HAL::SPIDevice *dev, uint8_t &id)
{
    if (dev == nullptr) {
        id = 0;
        return false;
    }

    WITH_SEMAPHORE(dev->get_semaphore());
    // Force bank 0 before reading the clone's WHO_AM_I register.
    return dev->write_register(0x7F, 0x00, false) &&
           dev->read_registers(WHO_AM_I_REG, &id, 1);
}

void setup()
{
    hal.gpio->pinMode(LED0_PIN, HAL_GPIO_OUTPUT);
    hal.gpio->pinMode(LED1_PIN, HAL_GPIO_OUTPUT);
    set_leds(false, false);

    imu1 = hal.spi->get_device("icm42688_1"); // SPI1
    imu2 = hal.spi->get_device("icm42688_2"); // SPI4
    configure_device(imu1.get());
    configure_device(imu2.get());

    hal.console->printf("\nLEVIA_H7 dual-IMU WHO_AM_I test\n");
    hal.console->printf("Expected value for both sensors: 0x%02X\n", EXPECTED_ID);
}

void loop()
{
    static bool blink_on;
    uint8_t id1 = 0;
    uint8_t id2 = 0;
    const bool transfer1 = read_whoami(imu1.get(), id1);
    const bool transfer2 = read_whoami(imu2.get(), id2);
    const bool pass1 = transfer1 && id1 == EXPECTED_ID;
    const bool pass2 = transfer2 && id2 == EXPECTED_ID;

    hal.console->printf("SPI1 IMU1: %s WHO_AM_I=0x%02X | SPI4 IMU2: %s WHO_AM_I=0x%02X\n",
                        pass1 ? "PASS" : (transfer1 ? "WRONG" : "READ_FAIL"), id1,
                        pass2 ? "PASS" : (transfer2 ? "WRONG" : "READ_FAIL"), id2);

    // LED0 represents SPI1 and LED1 represents SPI4. Each passing sensor
    // blinks its LED; a failed sensor leaves its LED off.
    blink_on = !blink_on;
    set_leds(pass1 && blink_on, pass2 && blink_on);
    hal.scheduler->delay(500);
}

AP_HAL_MAIN();
