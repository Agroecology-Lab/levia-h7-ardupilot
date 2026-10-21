#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 /path/to/ardupilot" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
cd "$ARDUPILOT_DIR"

./waf configure --board LEVIA_H7 --bootloader
./waf clean
./waf bootloader

echo "Bootloader BIN: $ARDUPILOT_DIR/build/LEVIA_H7/bin/AP_Bootloader.bin"
echo "Bootloader HEX: $ARDUPILOT_DIR/build/LEVIA_H7/bin/AP_Bootloader.hex"

