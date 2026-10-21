# Validated firmware images

This directory contains known-good firmware built for the provisional LEVIA H7
ArduPilot board ID `1224`, which current ArduPilot master has since allocated to
another board. Do not mix these images with a bootloader for a different ID.

## LEVIA_H7-24b18174

Built from ArduPilot source identity `24b18174` and bench-validated on the
dual-BMI270 LEVIA H7 board.

- `LEVIA_H7_arducopter.apj`: normal firmware update through the ArduPilot USB
  bootloader. Use this for routine upgrades.
- `LEVIA_H7_arducopter_with_bl.hex`: combined bootloader and ArduCopter image
  for initial programming or recovery with ST-Link. This overwrites both the
  bootloader and application regions.

SHA-256 checksums:

```text
c5494a19749a11cd18821cb47e92b4c528072e32cd8514578445687bcce0e8e6  LEVIA_H7_arducopter.apj
bb555f2a694cf70bb36c7a279cf47a9626eccbddac0cc0f95a07503336de19e5  LEVIA_H7_arducopter_with_bl.hex
```

These images are bring-up builds, not flight-qualified releases. Confirm the
board identity, sensor health, calibrations, output mapping, and all pre-arm
checks before operating motors with propellers fitted.
