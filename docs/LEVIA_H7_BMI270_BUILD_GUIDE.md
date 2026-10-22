# LEVIA H7 Dual-BMI270 ArduPilot Build and Bring-up Guide

## Validated configuration

This guide records the working configuration for the modified LEVIA H7 flight controller fitted with two genuine Bosch BMI270 inertial sensors. It covers the hardware rework, ArduPilot hardware definition, firmware build and upload, QGroundControl configuration, and the checks completed during bring-up.

Validated hardware and software:

- MCU: STM32H743
- IMU 1: Bosch BMI270 on SPI1
- IMU 2: Bosch BMI270 on SPI4
- Barometer: DPS368/DPS310-compatible device on internal I2C2
- Compass: IST8310 on internal I2C2
- Firmware: ArduCopter 4.8.0-dev at source revision `24b18174`
- Provisional private board type: `1224` / `AP_HW_LEVIA_H7`
- USB upload port used during validation: `/dev/cu.usbmodem101`

> Safety and release warning: this is a successful bench bring-up, not a flight qualification. Board ID `1224` is provisional until reserved in ArduPilot's official registry. Complete vibration, power, failsafe, motor-direction, control-direction, range, and restrained propulsion tests before flight.

## 1. Hardware rework

### Replace both IMUs

Replace the two installed IMUs with genuine Bosch BMI270 devices. Observe the package pin-1 marker and the PCB footprint orientation carefully.

The two positions are electrically separate:

- BMI270 1 uses SPI1.
- BMI270 2 uses SPI4.
- The second package is physically rotated relative to the first; firmware compensates with `ROTATION_YAW_90`.

### Secondary-interface pins

On the modified boards, package pins 2, 3, 10, and 11 were isolated from the
previous ground connections. These are BMI270 secondary-interface/OIS pins
(ASDX, ASCX, OCSB, and OSDO), not generic ground pins. In this SPI-only design
they are unused and were left electrically unconnected. They must not remain
shorted to ground or to each other.

Package pin 9 is BMI270 INT2. LEVIA routes the two INT2 signals to MCU pins PD14 and PD15. The native ArduPilot BMI270 backend does not require them, so firmware configures both MCU pins as high-impedance inputs.

### Pre-power checks

Before connecting USB or a battery supply:

1. Inspect every package edge for bridges and incomplete wetting.
2. Verify unused secondary-interface pins 2, 3, 10, and 11 are isolated.
3. Verify continuity from each SPI signal and chip-select net to the appropriate sensor pad.
4. Check resistance from 3.3 V to ground for an obvious short.
5. Power with a current-limited bench supply first.
6. Confirm both sensor supply pins measure approximately 3.3 V.

## 2. Final ArduPilot hardware definition

The working file is:

`libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/hwdef.dat`

### SPI and interrupt pins

```text
PD14 BMI270_1_INT2 INPUT
PD15 BMI270_2_INT2 INPUT

PA5  SPI1_SCK  SPI1
PA6  SPI1_MISO SPI1
PD7  SPI1_MOSI SPI1
PC15 IMU1_CS   CS
PB2  IMU1_DRDY INPUT

PE12 SPI4_SCK  SPI4
PE13 SPI4_MISO SPI4
PE14 SPI4_MOSI SPI4
PE11 IMU2_CS   CS
PE15 IMU2_DRDY INPUT
```

### Native BMI270 devices and probes

```text
SPIDEV bmi270_1 SPI1 DEVID1 IMU1_CS MODE3 1*MHZ 10*MHZ
SPIDEV bmi270_2 SPI4 DEVID1 IMU2_CS MODE3 1*MHZ 10*MHZ

DMA_NOSHARE SPI1* SPI4*

IMU BMI270 SPI:bmi270_1 ROTATION_NONE
IMU BMI270 SPI:bmi270_2 ROTATION_YAW_90
define HAL_DEFAULT_INS_FAST_SAMPLE 1
```

This uses ArduPilot's native `AP_InertialSensor_BMI270` FIFO backend. No custom BMI270 driver is required.

### Compass orientation

The validated onboard compass probe is:

```text
define AP_COMPASS_IST8310_INTERNAL_BUS_PROBING_ENABLED 0
COMPASS IST8310 I2C:0:0x0E false ROTATION_ROLL_180
define HAL_I2C_INTERNAL_MASK 1
define AP_COMPASS_PROBING_ENABLED 1
```

Disabling the generic internal IST8310 auto-probe prevents it from claiming the compass first with an incorrect default rotation.

## 3. Confirm both sensors before installing ArduCopter

The saved diagnostic is located at:

`Tools/LEVIA_H7/dual_bmi270_whoami_test/`

The BMI270 CHIP_ID register is `0x00`; the expected value is `0x24`. BMI270 SPI register reads require a dummy byte after the read address. The validated test returned:

```text
SPI1 BMI270: PASS CHIP_ID=0x24
SPI4 BMI270: PASS CHIP_ID=0x24
```

If either bus returns `0x00`, `0xFF`, or an unstable value, stop and repair the hardware before proceeding. Check CS, clock, MOSI, MISO, power, package alignment, and the unused secondary-interface pins.

## 4. Build ArduCopter

Open a terminal in the ArduPilot repository root:

```sh
cd /path/to/ardupilot
```

Configure and build with the local ARM toolchain:

```sh
env PATH="$PWD/.venv/bin:$PWD/.toolchains/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH" \
  ./waf configure --board LEVIA_H7

env PATH="$PWD/.venv/bin:$PWD/.toolchains/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH" \
  ./waf copter
```

The uploadable image is:

```text
build/LEVIA_H7/bin/arducopter.apj
```

The validated build used 1,436,104 bytes of flash and left 267,828 bytes free.

## 5. Upload on macOS

Quit QGroundControl before uploading so it releases the serial port.

Check the device name:

```sh
ls /dev/cu.usbmodem*
```

Upload:

```sh
cd /path/to/ardupilot
.venv/bin/python Tools/scripts/uploader.py \
  --port /dev/cu.usbmodem101 \
  build/LEVIA_H7/bin/arducopter.apj
```

If the uploader waits for the bootloader, press and release the board's BOOT button once. This LEVIA revision has no accessible NRST signal; NRST is not required for the upload procedure. A successful upload ends with erase, program, and verify at 100%, followed by `Rebooting.`

The USB modem number may change after reconnecting. Always run `ls /dev/cu.usbmodem*` rather than assuming it remains `101`.

## 6. QGroundControl configuration

### Board orientation

Set:

```text
AHRS_ORIENTATION = Yaw270
```

The ArduCopter build defaults to this; stored values override it. Reboot after changing it. Then perform a full accelerometer calibration with this final orientation selected. Do not compensate for a board-orientation problem by changing the individual IMU rotations: the two per-sensor rotations were independently validated against each other.

Expected motion checks:

- Nose up changes pitch smoothly and leaves roll close to zero.
- Right side up / left side down changes roll smoothly and leaves pitch close to zero.
- Clockwise yaw changes heading in the correct direction.

### Compass

Calibrate the compass after installing firmware containing `ROTATION_ROLL_180`. Move away from motors, speakers, steel benches, high-current wiring, and bench power supplies.

The successful result showed the IST8310 quality indicator in the green region and no `internal bad orientation` message. Reboot after calibration and compare the four cardinal headings against a trusted compass.

### Accelerometer and level

1. Run the full six-position accelerometer calibration.
2. Place the board in its intended installed attitude.
3. Run Level Horizon.
4. Reboot and verify the artificial horizon follows pitch and roll correctly.

## 7. Validated sensor results

### Native ArduPilot device IDs

The following parameter values identify two native BMI270 devices:

| Sensor | Acc/Gyro device ID | Decoded bus | Device type |
|---|---:|---|---|
| IMU0 | 3670282 | SPI1 | BMI270 (`0x38`) |
| IMU1 | 3670306 | SPI4 | BMI270 (`0x38`) |

`INS_ACC_ID` and `INS_GYR_ID` matched for IMU0. `INS_ACC2_ID` and `INS_GYR2_ID` matched for IMU1.

### Dynamic agreement test

A 20-second hand-motion capture collected approximately 1,019 samples from each IMU. After applying the configured rotations, both sensors agreed closely:

| Signal | X correlation | Y correlation | Z correlation |
|---|---:|---:|---:|
| Accelerometer | 0.9981 | 0.9975 | 0.9998 |
| Gyroscope | 0.9999 | 0.9998 | 0.9992 |

These results confirm the SPI routing, native driver operation, axis mapping, signs, and relative sensor rotations.

### Healthy boot signature

The final boot produced:

```text
EKF3 IMU1 initialised
EKF3 IMU0 initialised
EKF3 IMU1 tilt alignment complete
EKF3 IMU0 tilt alignment complete
EKF3 IMU1 MAG0 initial yaw alignment complete
EKF3 IMU0 MAG0 initial yaw alignment complete
AHRS: EKF3 active
```

No gyro, accelerometer, compass, AHRS, or EKF errors remained.

The remaining bench warnings were expected:

- `Battery 1 low voltage failsafe`: power the battery input at a valid voltage or configure the battery monitor for the test setup.
- `RC not found`: bind/connect the receiver before arming or temporarily configure bench-test arming checks as appropriate.

## 8. Troubleshooting checklist

### Sensor not detected

- Confirm 3.3 V at the BMI270.
- Confirm CHIP_ID `0x24` with the saved dual-bus test.
- Confirm SPI mode 3.
- Include the required dummy byte on SPI register reads.
- Check the correct CS net and MCU SPI pin mapping.
- Check unused secondary-interface pins 2, 3, 10, and 11 are floating.
- Check pin 9 is not driven as the former ICM CLKIN output.

### Axes swapped or inverted

- First confirm `AHRS_ORIENTATION = Yaw270`.
- Reboot and recalibrate the accelerometers after changing board orientation.
- Do not alter the validated IMU probe rotations unless the physical package orientation changes.
- Use MAVLink Inspector to plot `SCALED_IMU` and `SCALED_IMU2` independently.

### Compass calibration fails

- Confirm the firmware contains `ROTATION_ROLL_180`.
- Confirm generic IST8310 internal auto-probing is disabled.
- Reboot after a failed calibration before retrying.
- Calibrate away from magnetic interference.
- A `No location, fix_radius skipped` line is not itself the failure cause.
- Treat `internal bad orientation` as an orientation configuration error, not merely poor calibration motion.

## 9. Final pre-flight work still required

- Assign a unique ArduPilot board ID and produce a release bootloader.
- Verify power-rail behavior over the full battery range and under load.
- Complete current and voltage sensor calibration.
- Verify SD logging and review vibration/IMU clipping logs.
- Validate all motor numbers, directions, DShot behavior, and failsafe shutdown.
- Validate ELRS link loss, arming logic, and flight-mode selection.
- Recheck compass behavior with motors powered and perform CompassMot if needed.
- Perform restrained propulsion tests, then a cautious first hover in a clear area.

---

Documented from the successful LEVIA H7 dual-BMI270 bench bring-up, August 2026.
