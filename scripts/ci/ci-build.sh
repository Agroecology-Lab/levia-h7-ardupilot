#!/bin/sh
# Full integration build used by CI: install twice, build, verify.
# usage: ci-build.sh /path/to/ardupilot copter|rover [outdir]
set -eu

if [ "$#" -lt 2 ]; then
    echo "usage: $0 /path/to/ardupilot copter|rover [outdir]" >&2
    exit 2
fi

ARDUPILOT_DIR=$1
VEHICLE=$2
REPO_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)

sh "$REPO_DIR/scripts/install-into-ardupilot.sh" "$ARDUPILOT_DIR"

# a second run must change nothing
second=$(sh "$REPO_DIR/scripts/install-into-ardupilot.sh" "$ARDUPILOT_DIR")
if printf '%s\n' "$second" | grep -q '^applied '; then
    echo "error: install script is not idempotent:" >&2
    printf '%s\n' "$second" >&2
    exit 1
fi

sh "$REPO_DIR/scripts/build-$VEHICLE.sh" "$ARDUPILOT_DIR"
sh "$REPO_DIR/scripts/ci/verify-build.sh" "$ARDUPILOT_DIR" "$VEHICLE" ${3:+"$3"}
