# LEVIA H7 ArduPilot Support

Board support, diagnostics, and bring-up notes for the custom LEVIA H7 flight
controller.

The validated configuration uses an STM32H743 and two genuine Bosch BMI270
IMUs on SPI1 and SPI4. It builds against an upstream ArduPilot checkout; this
repository intentionally does not duplicate the full ArduPilot source tree.

## Repository layout

- `ardupilot/hwdef/LEVIA_H7/`: ChibiOS board and bootloader definitions.
- `ardupilot/examples/`: standalone hardware diagnostics.
- `ardupilot/tools/`: host-side diagnostic scripts.
- `bootloader/`: validated LEVIA bootloader images.
- `firmware/`: versioned, known-good USB-update and full ST-Link images.
- `docs/`: build, hardware-rework, calibration, and validation notes.
- `experiments/hxy42688/`: archived experimental support for the undocumented
  HXY IMU; not used by the current BMI270 flight firmware.
- `scripts/`: installation and build helpers for an ArduPilot checkout.

## Status

Bench validated:

- Dual BMI270 detection and native ArduPilot operation
- SPI1/SPI4 sensor agreement and orientation
- DPS368/DPS310-compatible barometer and IST8310 compass
- SD card, eight PWM/DShot outputs, ELRS, GPS, battery sensing, analog OSD,
  camera input, and switched VTX supply

This is not flight qualification. The private build uses a provisional LEVIA
board ID (see `ardupilot/BOARD_ID`); reserve one with ArduPilot before
distributing firmware. Also
complete the safety checks described in the guide.

## Quick start

Clone ArduPilot separately, including its submodules, then run:

```sh
./scripts/install-into-ardupilot.sh /path/to/ardupilot
./scripts/build-copter.sh /path/to/ardupilot   # or build-rover.sh
```

The install script registers the board ID, applies the Hiwonder EMM driver
patches from `ardupilot/patches/` and copies the hwdef. The build scripts
build the bootloader first if it is missing, or if the board ID or
`hwdef-bl.dat` changed since it was built.
Rerunning either script is safe.

The resulting firmware is:

```text
/path/to/ardupilot/build/LEVIA_H7/bin/arducopter.apj
```

## CI

GitHub Actions lints the repo, tests the install script and builds
bootloader, ArduCopter and ArduRover for LEVIA_H7. See `docs/CI.md`.

## First setup in QGroundControl

**Remove all propellers before configuration or motor testing.** Do not fly
immediately after flashing.

1. Connect the board, open **Vehicle Setup > Parameters**, and set
   `AHRS_ORIENTATION` to `Yaw270`. Save and reboot.
2. Run the full **Accelerometer** calibration, then **Level Horizon**, with the
   board installed in its final position.
3. Calibrate the **Compass** outdoors and away from motors, speakers, steel
   benches, power supplies, and high-current cables. Reboot afterward.
4. Bind the ELRS receiver and run **Radio** calibration. For the validated
   radio layout, channel 6 is Arm/Disarm and channel 7 selects flight modes.
5. Set `MOT_PWM_TYPE` to the protocol supported by the ESC (the tested ESC used
   `DShot300`). With no propellers fitted, verify every motor number and
   direction individually.
6. Configure the battery capacity and failsafe values. Compare QGroundControl
   voltage and current against trusted meter readings before relying on them.
7. Confirm GPS lock, SD-card logging, both IMUs, barometer, compass, RC link,
   and all pre-arm checks. Resolve every unexplained warning before flight.

Calibration values, compass offsets, radio endpoints, battery corrections,
and vehicle tuning are specific to each aircraft. Do not copy them from
another LEVIA unit. Start flight testing with a cautious low hover in a clear
area.

See [`docs/LEVIA_H7_BMI270_BUILD_GUIDE.md`](docs/LEVIA_H7_BMI270_BUILD_GUIDE.md)
for the full procedure.

Additional references:

- [Flashing guide](docs/FLASHING_GUIDE.md)
- [Connector pinout](docs/CONNECTOR_PINOUT.md)
- [Validated hardware](docs/VALIDATED_HARDWARE.md)

## Upstream relationship and licensing

The files under `ardupilot/` are intended to be copied into a compatible
ArduPilot source checkout. Derived ArduPilot code is distributed under GPL-3.0;
see `LICENSE` and the copyright notices in individual source files.
