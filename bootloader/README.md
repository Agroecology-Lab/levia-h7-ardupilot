# Bootloader images

`LEVIA_H7_bl.bin` and `LEVIA_H7_bl.hex` are USB-only ArduPilot bootloader
builds generated from `ardupilot/hwdef/LEVIA_H7/hwdef-bl.dat`.

These images must be rebuilt after changing the board ID. The private build
uses provisional LEVIA ID `1224`; reserve it upstream before distribution.
The ID-1224 build has compiled successfully but has not yet been flashed and
validated on hardware.
