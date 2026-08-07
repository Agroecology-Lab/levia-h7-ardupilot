# LEVIA H7 flashing guide

This guide applies to the dual-BMI270 LEVIA H7 build with provisional
ArduPilot board ID `1224`.

## Choose the correct file

| Situation | File |
|---|---|
| Blank MCU, damaged bootloader, or full recovery with ST-Link | `firmware/LEVIA_H7-24b18174/LEVIA_H7_arducopter_with_bl.hex` |
| Working LEVIA bootloader and normal USB update | `firmware/LEVIA_H7-24b18174/LEVIA_H7_arducopter.apj` |

The combined HEX overwrites both bootloader and application firmware. The APJ
updates only the application and cannot initialize a blank MCU.

## A. First programming or recovery with ST-Link

1. Remove propellers and disconnect motors and other high-power loads.
2. Power the flight controller normally from USB. Connect the ST-Link ground to
   board ground.
3. Connect `SWDIO`, `SWCLK`, `GND`, and the ST-Link voltage-sense/target-voltage
   input to the board's `3V3_MCU` rail. Do not inject a second supply into an
   already powered rail.
4. `NRST` is not exposed on this LEVIA revision and may be omitted.
5. In STM32CubeProgrammer select **ST-LINK**, **SWD**, and a conservative clock
   such as 1000 kHz if the default connection is unreliable.
6. Program `LEVIA_H7_arducopter_with_bl.hex`. Its addresses are already stored
   in the HEX file; do not manually relocate it.
7. Require erase, program, and verify to complete successfully, then disconnect
   the ST-Link and reconnect USB.

The board should enumerate as an ArduPilot USB device and identify itself as
`LEVIA_H7`, board type `1224`.

## B. Routine USB update

Quit QGroundControl first so it releases the serial port.

On macOS, find the current port:

```sh
ls /dev/cu.usbmodem*
```

From an ArduPilot checkout, upload the repository APJ:

```sh
cd /path/to/ardupilot
.venv/bin/python Tools/scripts/uploader.py \
  --port /dev/cu.usbmodemXXXX \
  /path/to/levia-h7-ardupilot/firmware/LEVIA_H7-24b18174/LEVIA_H7_arducopter.apj
```

Replace `XXXX` with the port reported by macOS. If the uploader remains at
`waiting for the bootloader`, press and release the LEVIA BOOT button once or
reconnect USB. A successful upload reports board type `1224`, then erase,
program, and verify at 100 percent followed by `Rebooting.`

## After flashing

1. Open QGroundControl and confirm the board is recognized as LEVIA H7.
2. Follow the first-setup checklist in the main README.
3. Resolve every unexplained sensor or pre-arm error before flight.

Do not load MatekH743 firmware or a board-ID-1013 APJ onto this LEVIA
bootloader. Do not fly immediately after programming.

