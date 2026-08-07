# M1-M4 output identification diagnostic

This example drives only motor GPIO outputs 1 through 4. M1 emits one pulse,
M2 two pulses, M3 three pulses, and M4 four pulses.

Disconnect ESCs and remove all propellers before installing or running this
diagnostic.

```sh
./waf configure --board LEVIA_H7
./waf build --targets examples/LEVIA_motor_pattern
```

