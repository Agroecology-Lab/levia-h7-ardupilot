#!/bin/sh
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 /path/to/ardupilot" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
REPO_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)

if [ ! -x "$ARDUPILOT_DIR/waf" ] || [ ! -d "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS" ]; then
    echo "error: $ARDUPILOT_DIR is not an ArduPilot checkout" >&2
    exit 1
fi

if ! grep -q '^AP_HW_LEVIA_H7[[:space:]]' "$ARDUPILOT_DIR/Tools/AP_Bootloader/board_types.txt"; then
    patch -d "$ARDUPILOT_DIR" -p1 < "$REPO_DIR/ardupilot/board_types.patch"
fi

mkdir -p "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7"
mkdir -p "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_motor_pattern"
mkdir -p "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_bmi270_whoami"

cp "$REPO_DIR/ardupilot/hwdef/LEVIA_H7/hwdef.dat" \
   "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/hwdef.dat"
cp "$REPO_DIR/ardupilot/hwdef/LEVIA_H7/hwdef-bl.dat" \
   "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/hwdef-bl.dat"
cp "$REPO_DIR/ardupilot/examples/LEVIA_motor_pattern/LEVIA_motor_pattern.cpp" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_motor_pattern/LEVIA_motor_pattern.cpp"
cp "$REPO_DIR/ardupilot/examples/LEVIA_motor_pattern/wscript" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_motor_pattern/wscript"
cp "$REPO_DIR/ardupilot/examples/LEVIA_bmi270_whoami/LEVIA_bmi270_whoami.cpp" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_bmi270_whoami/LEVIA_bmi270_whoami.cpp"
cp "$REPO_DIR/ardupilot/examples/LEVIA_bmi270_whoami/wscript" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_bmi270_whoami/wscript"

echo "Installed LEVIA H7 support into $ARDUPILOT_DIR"
