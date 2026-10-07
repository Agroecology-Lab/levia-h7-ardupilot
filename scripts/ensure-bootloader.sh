#!/bin/sh
# Build the LEVIA_H7 bootloader into Tools/bootloaders/ if it is missing or
# was built for a different board ID. waf refuses to configure the board
# without it.
set -eu

if [ "$#" -ne 1 ]; then
    echo "usage: $0 /path/to/ardupilot" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
REPO_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BOARD_ID=$(tr -d ' \t\r\n' < "$REPO_DIR/ardupilot/BOARD_ID")

cd "$ARDUPILOT_DIR"
BIN=Tools/bootloaders/LEVIA_H7_bl.bin
STAMP=Tools/bootloaders/LEVIA_H7_bl.stamp

# rebuild when the board ID or the bootloader hwdef changes
WANT="$BOARD_ID $(sha256sum libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/hwdef-bl.dat | cut -d' ' -f1)"
if [ -f "$BIN" ] && [ -f "$STAMP" ] && [ "$(cat "$STAMP")" = "$WANT" ]; then
    exit 0
fi

echo "Building LEVIA_H7 bootloader for board ID $BOARD_ID"
mkdir -p Tools/bootloaders
./waf configure --board LEVIA_H7 --bootloader
# clean first, as ArduPilot does: a build dir left by a firmware build breaks
# the bootloader build with stale generated DroneCAN headers
./waf clean
./waf bootloader
cp build/LEVIA_H7/bin/AP_Bootloader.bin "$BIN"
echo "$WANT" > "$STAMP"
