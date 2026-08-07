# Validated hardware

The following hardware was exercised during the LEVIA H7 bench bring-up. This
is a record of successful interoperability, not a flight qualification or a
guarantee that every similar product is compatible.

| Item | Tested hardware | Result |
|---|---|---|
| Flight-controller MCU | STM32H743 | Bootloader, USB, ArduCopter, storage, and outputs operational |
| IMU 1 | Genuine Bosch BMI270 on SPI1 | Native ArduPilot driver; CHIP_ID `0x24`; dynamic data validated |
| IMU 2 | Genuine Bosch BMI270 on SPI4 | Native ArduPilot driver; CHIP_ID `0x24`; dynamic agreement with IMU1 validated |
| Barometer | DPS368/DPS310-compatible device on internal I2C2 | Detected and calibrated |
| Onboard compass | IST8310 on internal I2C2 | Detected, orientation corrected, and calibrated |
| 4-in-1 ESC | MicoAir AM32-4IN1-55A, AM32 firmware | DShot300 and individual motor outputs validated; 12.75 mV/A current output used |
| Motor | 2307-class brushless motor, no propeller | Individual spin test completed |
| RC receiver | ExpressLRS/CRSF receiver on UART6 (`U8`) | Bound, radio calibrated, 250 Hz link and telemetry reported |
| GPS | MatekSys SAM-M8Q on UART2 (`U24`) | u-blox detected, protocol 18.00, 3D fix obtained |
| External GPS compass | QMC5883 on the SAM-M8Q module | Electrical connection available through I2C1; not separately qualified in this record |
| Analog video | Analog camera on `U11`; VTX1600-class transmitter on `U10` | Clean video through AT7456E OSD; UART1 TX/IRC control and switched 9 V rail exercised |
| Storage | microSD card in onboard slot | FATFS logging verified |
| Ground station | QGroundControl on Apple-silicon macOS | Board recognition, calibration, MAVLink inspection, and motor tests completed |
| Debug/programmer | ST-Link MB1762B / STLINK-V3MINIE-class SWD connection | Full combined-HEX programming without NRST validated |

The exact ELRS receiver, camera, motor, and microSD model numbers were not
recorded. Treat replacements as new compatibility tests. The original
undocumented HXY/TDK-lookalike IMUs are not part of the validated flight build.

