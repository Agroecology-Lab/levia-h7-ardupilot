#!/bin/sh
# Check a finished LEVIA_H7 build and collect release-style artifacts.
# usage: verify-build.sh /path/to/ardupilot copter|rover [outdir]
set -eu

if [ "$#" -lt 2 ]; then
    echo "usage: $0 /path/to/ardupilot copter|rover [outdir]" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
VEHICLE=$2
OUT=${3:-$ARDUPILOT_DIR/build/LEVIA_H7/ci-out}
MIN_FREE_KB=${MIN_FREE_KB:-128}
REPO_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BOARD_ID=$(tr -d ' \t\r\n' < "$REPO_DIR/ardupilot/BOARD_ID")

case "$VEHICLE" in
    copter|rover) ELF_NAME=ardu$VEHICLE ;;
    *) echo "error: unknown vehicle $VEHICLE" >&2; exit 2 ;;
esac

BUILD=$ARDUPILOT_DIR/build/LEVIA_H7
BIN=$BUILD/bin
for f in "$BIN/$ELF_NAME" "$BIN/$ELF_NAME.apj" "$BIN/$ELF_NAME.bin" \
         "$ARDUPILOT_DIR/Tools/bootloaders/LEVIA_H7_bl.bin"; do
    if [ ! -s "$f" ]; then
        echo "error: missing or empty $f" >&2
        exit 1
    fi
done

# image identity and flash headroom come from the .apj
python3 - "$BIN/$ELF_NAME.apj" "$BOARD_ID" "$MIN_FREE_KB" <<'PY'
import json, sys
apj, board_id, min_free_kb = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
d = json.load(open(apj))
if d["board_id"] != board_id:
    sys.exit(f"error: image board_id {d['board_id']} != BOARD_ID {board_id}")
free_kb = d["flash_free"] // 1024
if free_kb < min_free_kb:
    sys.exit(f"error: only {free_kb} KB flash free, need {min_free_kb}")
print(f"board_id {d['board_id']}, {d['image_size']} bytes, {free_kb} KB flash free")
PY

# the EMM driver must really be in the image, not silently compiled out
if [ "$(arm-none-eabi-nm -C "$BIN/$ELF_NAME" | grep -c 'AP_HiwonderEMM')" -eq 0 ]; then
    echo "error: AP_HiwonderEMM missing from $ELF_NAME" >&2
    exit 1
fi

# every default in the repo's defaults.parm must reach the image
DEFAULTS=$REPO_DIR/ardupilot/hwdef/LEVIA_H7/defaults.parm
grep -v '^[[:space:]]*#' "$DEFAULTS" | grep -v '^[[:space:]]*$' > "$BUILD/expected_defaults.txt"
want=$(wc -l < "$BUILD/expected_defaults.txt")
have=$(grep -cFxf "$BUILD/expected_defaults.txt" "$BUILD/processed_defaults.parm" || true)
if [ "$want" -ne "$have" ]; then
    echo "error: $have of $want defaults.parm lines reached the image" >&2
    exit 1
fi

mkdir -p "$OUT"
cp "$BIN/$ELF_NAME.apj" "$BIN/$ELF_NAME.bin" "$OUT/"
cp "$ARDUPILOT_DIR/Tools/bootloaders/LEVIA_H7_bl.bin" "$OUT/"
{
    echo "ardupilot: $(git -C "$ARDUPILOT_DIR" rev-parse HEAD)"
    echo "board repo: $(git -C "$REPO_DIR" rev-parse HEAD 2>/dev/null || echo unknown)"
    echo "board id: $BOARD_ID"
    echo "vehicle: $VEHICLE"
    echo "toolchain: $(arm-none-eabi-gcc --version | head -1)"
} > "$OUT/build-info.txt"
(cd "$OUT" && sha256sum -- *.apj *.bin > SHA256SUMS)
echo "verified, artifacts in $OUT"
