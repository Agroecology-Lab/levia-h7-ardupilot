#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/SPIDevice.h>

const AP_HAL::HAL &hal = AP_HAL::get_HAL();

static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu1;
static AP_HAL::OwnPtr<AP_HAL::SPIDevice> imu2;

static constexpr uint8_t LED0_PIN = 90;
static constexpr uint8_t LED1_PIN = 91;
static constexpr uint8_t CHIP_ID_REG = 0x00;
static constexpr uint8_t EXPECTED_ID = 0x24;

void setup();
void loop();

static void set_leds(bool led0, bool led1)
{
    // LEVIA H7 LEDs are active low.
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

static bool read_chip_id(AP_HAL::SPIDevice *dev, uint8_t &id)
{
    if (dev == nullptr) {
        id = 0;
        return false;
    }

    WITH_SEMAPHORE(dev->get_semaphore());
    // BMI270 SPI reads return one dummy byte after the register address.
    uint8_t transfer[3] { uint8_t(CHIP_ID_REG | 0x80), 0, 0 };
    if (!dev->transfer_fullduplex(transfer, sizeof(transfer))) {
        id = 0;
        return false;
    }
    id = transfer[2];
    return true;
}

void setup()
{
    hal.gpio->pinMode(LED0_PIN, HAL_GPIO_OUTPUT);
    hal.gpio->pinMode(LED1_PIN, HAL_GPIO_OUTPUT);
    set_leds(false, false);

    imu1 = hal.spi->get_device("bmi270_1");
    imu2 = hal.spi->get_device("bmi270_2");
    configure_device(imu1.get());
    configure_device(imu2.get());

    hal.console->printf("\nLEVIA H7 dual-BMI270 CHIP_ID test\n");
    hal.console->printf("Expected value for both sensors: 0x%02X\n", EXPECTED_ID);
}

void loop()
{
    static bool blink_on;
    uint8_t id1 = 0;
    uint8_t id2 = 0;
    const bool transfer1 = read_chip_id(imu1.get(), id1);
    const bool transfer2 = read_chip_id(imu2.get(), id2);
    const bool pass1 = transfer1 && id1 == EXPECTED_ID;
    const bool pass2 = transfer2 && id2 == EXPECTED_ID;

    hal.console->printf("SPI1 BMI270: %s CHIP_ID=0x%02X | SPI4 BMI270: %s CHIP_ID=0x%02X\n",
                        pass1 ? "PASS" : (transfer1 ? "WRONG" : "READ_FAIL"), id1,
                        pass2 ? "PASS" : (transfer2 ? "WRONG" : "READ_FAIL"), id2);

    blink_on = !blink_on;
    set_leds(pass1 && blink_on, pass2 && blink_on);
    hal.scheduler->delay(500);
}

AP_HAL_MAIN();
