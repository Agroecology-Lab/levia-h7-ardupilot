#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 /path/to/ardupilot" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
cd "$ARDUPILOT_DIR"

./waf configure --board LEVIA_H7
./waf build --targets examples/LEVIA_bmi270_whoami,examples/LEVIA_motor_pattern

echo "Diagnostics are under $ARDUPILOT_DIR/build/LEVIA_H7/bin"

