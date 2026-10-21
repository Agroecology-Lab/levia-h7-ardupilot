# Bootloader images

`LEVIA_H7_bl.bin` and `LEVIA_H7_bl.hex` are USB-only ArduPilot bootloader
builds generated from `ardupilot/hwdef/LEVIA_H7/hwdef-bl.dat`.

These images are for ID `1224` only. The current ID is in `ardupilot/BOARD_ID`
and `scripts/ensure-bootloader.sh` builds a matching bootloader. Reserve an ID
upstream before distribution.
The ID-1224 build has compiled successfully but has not yet been flashed and
validated on hardware.
