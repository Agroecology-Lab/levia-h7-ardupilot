# Dual-BMI270 CHIP_ID diagnostic

Reads the Bosch BMI270 `CHIP_ID` register on LEVIA SPI1 and SPI4. Both devices
must return `0x24`. LED0 represents SPI1 and LED1 represents SPI4; a passing
sensor blinks its corresponding active-low LED.

```sh
./waf configure --board LEVIA_H7
./waf build --targets examples/LEVIA_bmi270_whoami
```

The uploadable image is
`build/LEVIA_H7/bin/LEVIA_bmi270_whoami.apj`.

