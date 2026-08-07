#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 /path/to/ardupilot" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
cd "$ARDUPILOT_DIR"

./waf configure --board LEVIA_H7
./waf copter

echo "Firmware: $ARDUPILOT_DIR/build/LEVIA_H7/bin/arducopter.apj"

