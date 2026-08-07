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

This is not flight qualification. The private build uses provisional LEVIA
board ID `1224`; reserve it with ArduPilot before distributing firmware. Also
complete the safety checks described in the guide.

## Quick start

Clone ArduPilot separately, including its submodules, then run:

```sh
./scripts/install-into-ardupilot.sh /path/to/ardupilot
./scripts/build-copter.sh /path/to/ardupilot
```

The resulting firmware is:

```text
/path/to/ardupilot/build/LEVIA_H7/bin/arducopter.apj
```

See [`docs/LEVIA_H7_BMI270_BUILD_GUIDE.md`](docs/LEVIA_H7_BMI270_BUILD_GUIDE.md)
for the full procedure.

## Upstream relationship and licensing

The files under `ardupilot/` are intended to be copied into a compatible
ArduPilot source checkout. Derived ArduPilot code is distributed under GPL-3.0;
see `LICENSE` and the copyright notices in individual source files.
