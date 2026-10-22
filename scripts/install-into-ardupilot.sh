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

# Board ID. Kept in one file so a clash with an upstream allocation is a
# one-line change. The bootloader embeds the ID, so changing it needs a
# bootloader rebuild (done automatically) and an ST-Link reflash.
BOARD_ID=$(tr -d ' \t\r\n' < "$REPO_DIR/ardupilot/BOARD_ID")
case "$BOARD_ID" in
    ''|*[!0-9]*) echo "error: ardupilot/BOARD_ID must be a number" >&2; exit 1 ;;
esac

BOARD_TYPES="$ARDUPILOT_DIR/Tools/AP_Bootloader/board_types.txt"
OWNER=$(awk -v id="$BOARD_ID" '$1 ~ /^AP_HW_/ && $2 == id { print $1; exit }' "$BOARD_TYPES")
CURRENT=$(awk '$1 == "AP_HW_LEVIA_H7" { print $2; exit }' "$BOARD_TYPES")

if [ "$OWNER" = "AP_HW_LEVIA_H7" ]; then
    :   # already registered with this ID
elif [ -n "$OWNER" ]; then
    echo "error: board ID $BOARD_ID is already used by $OWNER in this ArduPilot checkout." >&2
    echo "       Pick an unused ID in ardupilot/BOARD_ID." >&2
    exit 1
elif [ -n "$CURRENT" ]; then
    echo "error: AP_HW_LEVIA_H7 is already defined as $CURRENT in $BOARD_TYPES." >&2
    echo "       Remove that line or set ardupilot/BOARD_ID to $CURRENT." >&2
    exit 1
else
    # insert after the entry with the largest ID below ours, outside the ODID range
    TMP=$(mktemp)
    awk -v id="$BOARD_ID" '
        { line[NR] = $0
          if ($1 ~ /^AP_HW_/ && $2 ~ /^[0-9]+$/ && $2 + 0 < id && $2 + 0 >= best_id) { best = NR; best_id = $2 + 0 } }
        END {
            for (i = 1; i <= NR; i++) {
                print line[i]
                if (i == best) { printf "%-37s%s\n", "AP_HW_LEVIA_H7", id }
            }
        }' "$BOARD_TYPES" > "$TMP"
    if ! grep -q '^AP_HW_LEVIA_H7[[:space:]]' "$TMP"; then
        rm -f "$TMP"
        echo "error: could not insert AP_HW_LEVIA_H7 into $BOARD_TYPES" >&2
        exit 1
    fi
    cat "$TMP" > "$BOARD_TYPES"
    rm -f "$TMP"
fi

# Hiwonder EMM driver patches (idempotent). SKIP_EMM_PATCHES=1 is for tests.
if [ "${SKIP_EMM_PATCHES:-0}" != 1 ]; then
    for patch in "$REPO_DIR"/ardupilot/patches/*.patch; do
        if git -C "$ARDUPILOT_DIR" apply --check "$patch" 2>/dev/null; then
            git -C "$ARDUPILOT_DIR" apply "$patch"
            echo "applied $(basename "$patch")"
        elif git -C "$ARDUPILOT_DIR" apply --check -R "$patch" 2>/dev/null; then
            echo "already applied $(basename "$patch")"
        else
            echo "error: $(basename "$patch") neither applies nor is already applied." >&2
            echo "       This ArduPilot revision has drifted from the one the patches target." >&2
            exit 1
        fi
    done
fi

mkdir -p "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7"
mkdir -p "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_motor_pattern"
mkdir -p "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_bmi270_whoami"

for f in hwdef.dat hwdef-bl.dat defaults.parm; do
    cp "$REPO_DIR/ardupilot/hwdef/LEVIA_H7/$f" \
       "$ARDUPILOT_DIR/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/$f"
done
cp "$REPO_DIR/ardupilot/examples/LEVIA_motor_pattern/LEVIA_motor_pattern.cpp" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_motor_pattern/LEVIA_motor_pattern.cpp"
cp "$REPO_DIR/ardupilot/examples/LEVIA_motor_pattern/wscript" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_motor_pattern/wscript"
cp "$REPO_DIR/ardupilot/examples/LEVIA_bmi270_whoami/LEVIA_bmi270_whoami.cpp" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_bmi270_whoami/LEVIA_bmi270_whoami.cpp"
cp "$REPO_DIR/ardupilot/examples/LEVIA_bmi270_whoami/wscript" \
   "$ARDUPILOT_DIR/libraries/AP_HAL/examples/LEVIA_bmi270_whoami/wscript"

echo "Installed LEVIA H7 support (board ID $BOARD_ID) into $ARDUPILOT_DIR"
