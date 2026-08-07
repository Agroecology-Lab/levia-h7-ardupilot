/*
  LEVIA_H7 M1-M4 pin-identification burst pattern.

  WARNING: This deliberately drives motor output pins. Disconnect all ESCs
  and propellers before flashing or running this diagnostic.
*/

#include <AP_HAL/AP_HAL.h>

const AP_HAL::HAL &hal = AP_HAL::get_HAL();

void setup();
void loop();

static constexpr uint8_t MOTOR_COUNT = 4;
static constexpr uint8_t motor_gpio[MOTOR_COUNT] = {50, 51, 52, 53};
static constexpr uint16_t HIGH_MS = 100;
static constexpr uint16_t LOW_MS = 150;
static constexpr uint16_t CHANNEL_GAP_MS = 600;
static constexpr uint16_t CYCLE_GAP_MS = 1500;

void setup()
{
    hal.console->printf("LEVIA_H7 M1-M4 GPIO burst pattern; ESCs must be disconnected\n");
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        hal.gpio->pinMode(motor_gpio[i], HAL_GPIO_OUTPUT);
        hal.gpio->write(motor_gpio[i], false);
    }
}

void loop()
{
    for (uint8_t motor = 0; motor < MOTOR_COUNT; motor++) {
        const uint8_t pulse_count = motor + 1U;
        hal.console->printf("M%u: %u pulse(s)\n", unsigned(motor + 1U), unsigned(pulse_count));

        for (uint8_t pulse = 0; pulse < pulse_count; pulse++) {
            hal.gpio->write(motor_gpio[motor], true);
            hal.scheduler->delay(HIGH_MS);
            hal.gpio->write(motor_gpio[motor], false);
            hal.scheduler->delay(LOW_MS);
        }
        hal.scheduler->delay(CHANNEL_GAP_MS);
    }

    hal.console->printf("cycle pause\n");
    hal.scheduler->delay(CYCLE_GAP_MS);
}

AP_HAL_MAIN();
