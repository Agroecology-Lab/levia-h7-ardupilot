# LEVIA H7 connector pinout

This table is cross-checked against the LEVIA H7 schematic and the fabricated
PCB netlist. Always use the PCB pin-1 marker and connector reference. Do not
infer pin order from cable colors or from another flight controller.

**Electrical warnings**

- `VBAT` is direct battery voltage.
- `+9V` is a switched VTX rail. Fit a VTX antenna before enabling it.
- UART, I2C, SPI, CAN MCU-side, and motor-control signals use 3.3 V logic.
- The I2C1 and SPI3 connectors provide 5 V power but still use 3.3 V logic.
- Connector shield/anchor pads are tied to ground and are not additional
  signal positions.
- Verify VBAT and GND with a multimeter before connecting any 4-in-1 ESC.

## U8 - RC input / UART6

| Pin | Signal | Use |
|---:|---|---|
| 1 | UART6_TX | FC transmit to CRSF/ELRS receiver RX |
| 2 | UART6_RX | FC receive from CRSF/ELRS receiver TX |
| 3 | GND | Ground |
| 4 | +5V_SYS | Receiver power |

## U24 - GPS / UART2

| Pin | Signal | Use |
|---:|---|---|
| 1 | UART2_TX | FC transmit; connect to GPS RX |
| 2 | UART2_RX | FC receive; connect to GPS TX |
| 3 | GND | Ground |
| 4 | +5V_SYS | GPS power |

For a GPS/compass module, connect its SDA and SCL separately to `U18`.

## U10 - analog VTX

| Pin | Signal | Use |
|---:|---|---|
| 1 | VTX | OSD video output to transmitter |
| 2 | UART1_TX | One-way IRC Tramp/control output |
| 3 | GND | Ground |
| 4 | +9V | Switched transmitter power |

`U10` is physically reversed relative to `U11`; follow the pin-1 marker.

## U11 - analog camera

| Pin | Signal | Use |
|---:|---|---|
| 1 | VRX | Camera video into onboard OSD |
| 2 | GND | Ground |
| 3 | +5V_SYS | Camera power |

## U13 - CAN

| Pin | Signal | Use |
|---:|---|---|
| 1 | +5V_SYS | Peripheral power |
| 2 | GND | Ground |
| 3 | CAN1_L | CAN low |
| 4 | CAN1_H | CAN high |

The onboard 120-ohm termination is controlled by `JP1`. CAN was electrically
routed but was not part of the completed peripheral validation.

## U18 - external I2C1

| Pin | Signal | Use |
|---:|---|---|
| 1 | +5V_SYS | Peripheral power |
| 2 | GND | Ground |
| 3 | I2C1_SDA | 3.3 V logic data |
| 4 | I2C1_SCL | 3.3 V logic clock |

## U19 - external SPI3

| Pin | Signal | Use |
|---:|---|---|
| 1 | +5V_SYS | Peripheral power |
| 2 | GND | Ground |
| 3 | SPI3_SCLK | 3.3 V logic clock |
| 4 | SPI3_MISO | Peripheral to FC |
| 5 | SPI3_MOSI | FC to peripheral |
| 6 | SPI3_CS1 | Chip select 1 |

The PCB also routes a second SPI3 chip-select internally, but it is not present
on this six-position connector.

## U20 - digital video connector

| Pin | Signal | Use |
|---:|---|---|
| 1 | +9V | Switched digital-VTX power |
| 2 | GND | Ground |
| 3 | UART4_TX | FC transmit |
| 4 | UART4_RX | FC receive |
| 5 | GND | Ground |
| 6 | UART3_RX | Additional FC receive line |

This connector was routed for DJI O3/O4-style systems but was not validated in
the completed analog-video bring-up. Confirm the air-unit cable pinout before
connection.

## U21 - buzzer

| Pin | Signal | Use |
|---:|---|---|
| 1 | GND | Ground |
| 2 | +5V_SYS | Buzzer positive |
| 3 | BUZZ_PAD- | Switched buzzer negative |

## U22 - ESC / motors 1-4

| Pin | Signal | Use |
|---:|---|---|
| 1 | VBAT | Direct battery voltage |
| 2 | GND | Power ground |
| 3 | ADC_CURR1 | ESC analog current-sense output |
| 4 | UART8_RX | ESC telemetry input to FC |
| 5 | MOTOR1 / S1 | Motor output 1 |
| 6 | MOTOR2 / S2 | Motor output 2 |
| 7 | MOTOR3 / S3 | Motor output 3 |
| 8 | MOTOR4 / S4 | Motor output 4 |

The schematic contains a proposed `U23` connector for motors 5-8, but `U23`
is not fitted in the referenced PCB layout. Outputs 5-8 remain defined in
firmware and are available only through the corresponding board routing/pads.

## USB-C, microSD, and SWD

- USB-C provides USB data and board power; it is the normal firmware-update
  and QGroundControl connection.
- The onboard microSD slot uses SDMMC1 and is intended for ArduPilot logs.
- SWD programming uses `SWDIO`, `SWCLK`, `GND`, and `3V3_MCU` target-voltage
  sense. This board revision has no accessible NRST pad.

