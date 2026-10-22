#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 /path/to/ardupilot" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
REPO_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
sh "$REPO_DIR/scripts/ensure-bootloader.sh" "$ARDUPILOT_DIR"

cd "$ARDUPILOT_DIR"
# board defaults plus this vehicle's defaults, through the hwdef defaults.parm
{ cat "$REPO_DIR/ardupilot/hwdef/LEVIA_H7/defaults.parm"; echo; cat "$REPO_DIR/ardupilot/params/copter.parm"; } \
    > "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/defaults.parm"
./waf configure --board LEVIA_H7
./waf copter

echo "Firmware: $ARDUPILOT_DIR/build/LEVIA_H7/bin/arducopter.apj"

