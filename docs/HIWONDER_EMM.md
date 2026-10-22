# Hiwonder 4-channel encoder motor driver

Ported from the PX4 `hiwonder_emm` driver as `AP_HiwonderEMM`, enabled by
`define AP_HIWONDEREMM_ENABLED 1` in the hwdef.

Status: compiles for LEVIA_H7 (Rover). **Not yet run in SITL or on hardware.**
The I2C register map and speed scaling are copied from PX4 and unverified
against Hiwonder's documentation.

## Wiring

External I2C1 on the board is ArduPilot I2C bus 1, the parameter default. The
driver address defaults to 0x34. Check pull-ups and logic levels on the
motor board before connecting.

## Parameters

| Name | Default | Meaning |
| --- | --- | --- |
| `SERVO_EMM_MASK` | 0 | Servo channels sent to the board, lowest bit is motor 1, max 4. Reboot after changing. |
| `SERVO_EMM_BUS` | 1 | I2C bus |
| `SERVO_EMM_ADDR` | 52 (0x34) | I2C address |
| `SERVO_EMM_TYPE` | 3 | 0 none, 1 TT, 2 N20, 3 JGB37-520 12V 110RPM |
| `SERVO_EMM_POL` | 0 | Encoder polarity |

Channels in the mask stop producing PWM, and their `SERVOn_MIN/MAX/TRIM` are
forced to 1000/2000/1500. 1500 is stopped; 1000..2000 maps to speed -127..127
(PX4 used 0..255 minus 128, which was asymmetric).

For a skid-steer Rover, set `SERVO1`/`SERVO2` to throttleLeft and
`SERVO3`/`SERVO4` to throttleRight, then `SERVO_EMM_MASK` to 15.

## Behaviour added over PX4

- Motors are commanded to zero if the main loop stops updating for 200 ms.
  This does not cover a hung I2C bus.
- A missing board is retried once a second and blocks arming
  (`EMM not found on bus N`).
- Supply voltage and encoder counts are logged as `EMM` at 10 Hz.

## Bench test before driving

Wheels off the ground. Confirm the disarmed output is stopped, that each motor
follows the right servo channel and direction, and that motors stop when the
autopilot is reset or the I2C cable is pulled.
