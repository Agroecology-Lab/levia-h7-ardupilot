#!/bin/sh
# Tests install-into-ardupilot.sh against a fake ArduPilot tree. No network and
# no real checkout, so it runs in seconds. The patch step is skipped; the
# integration job exercises it against real ArduPilot.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO=$(CDPATH= cd -- "$HERE/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
export SKIP_EMM_PATCHES=1

fails=0
fail() { echo "FAIL: $*" >&2; fails=$((fails + 1)); }
ok() { echo "ok: $*"; }
# check "description" command args...: ok if the command succeeds
check() { desc=$1; shift; if "$@"; then ok "$desc"; else fail "$desc"; fi; }

# copy of this repo's installable parts with a chosen board ID
make_repo() {
    dir=$TMP/$1
    mkdir -p "$dir"
    cp -R "$REPO/scripts" "$REPO/ardupilot" "$dir/"
    printf '%s\n' "$2" > "$dir/ardupilot/BOARD_ID"
}

# fake ArduPilot checkout; $2 is extra lines for board_types.txt
make_ap() {
    dir=$TMP/$1
    mkdir -p "$dir/libraries/AP_HAL_ChibiOS" "$dir/Tools/AP_Bootloader"
    printf '#!/bin/sh\n' > "$dir/waf"
    chmod +x "$dir/waf"
    {
        printf '# board types\nAP_HW_HWH7                           1223\n\n'
        printf 'AP_HW_BrahmaH7                       1224\n\n'
        printf '# FlyingRC\nAP_HW_FLYINGRCH7DPRO                 1242\n\n'
        printf 'AP_HW_PRINCIPIOT_H7PI                1248\n\n'
        printf 'AP_HW_GPILOT_P1\t\t\t\t\t\t 1275\n\n'
        printf 'AP_HW_SIYI_N7_ODID                   11123\n'
        if [ -n "${2:-}" ]; then printf '%s\n' "$2"; fi
    } > "$dir/Tools/AP_Bootloader/board_types.txt"
}

run() { sh "$TMP/$1/scripts/install-into-ardupilot.sh" "$TMP/$2"; }

# 1. fresh install inserts the ID once, in order, with the file's column
make_repo r1 1245
make_ap a1
run r1 a1 > /dev/null 2> "$TMP/err1" || fail "fresh install failed: $(cat "$TMP/err1")"
BT=$TMP/a1/Tools/AP_Bootloader/board_types.txt
check "ID inserted once" test "$(grep -c '^AP_HW_LEVIA_H7 ' "$BT")" = 1
check "column alignment" grep -qx 'AP_HW_LEVIA_H7                       1245' "$BT"
# the line before the new entry must be the nearest lower ID (1242)
check "inserted after nearest lower ID" \
    test "$(grep -B1 '^AP_HW_LEVIA_H7 ' "$BT" | head -1)" = 'AP_HW_FLYINGRCH7DPRO                 1242'

# 2. hwdef files and examples land where waf expects them
for f in hwdef.dat hwdef-bl.dat defaults.parm; do
    [ -f "$TMP/a1/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/$f" ] || fail "$f not installed"
done
[ -f "$TMP/a1/libraries/AP_HAL/examples/LEVIA_motor_pattern/wscript" ] || fail "examples not installed"
ok "files installed"

# 3. rerun is a no-op
cp "$BT" "$TMP/bt.before"
run r1 a1 > /dev/null 2>&1 || fail "second run failed"
check "idempotent" cmp -s "$BT" "$TMP/bt.before"

# 4. ID owned by another board is refused and the file left alone
make_repo r2 1224
make_ap a2
cp "$TMP/a2/Tools/AP_Bootloader/board_types.txt" "$TMP/bt.a2"
if run r2 a2 > /dev/null 2> "$TMP/err2"; then
    fail "collision with AP_HW_BrahmaH7 was accepted"
else
    check "collision names the owner" grep -q 'AP_HW_BrahmaH7' "$TMP/err2"
    check "collision leaves file untouched" cmp -s "$TMP/a2/Tools/AP_Bootloader/board_types.txt" "$TMP/bt.a2"
fi

# 5. an existing LEVIA entry with a different ID is refused
make_ap a3 'AP_HW_LEVIA_H7                       1999'
if run r1 a3 > /dev/null 2> "$TMP/err3"; then
    fail "conflicting existing LEVIA ID accepted"
else
    ok "conflicting existing ID refused"
fi

# 6. junk BOARD_ID is refused
make_repo r4 12ab
make_ap a4
if run r4 a4 > /dev/null 2>&1; then fail "non-numeric BOARD_ID accepted"; else ok "non-numeric ID refused"; fi

# 7. a directory that is not ArduPilot is refused
mkdir "$TMP/notap"
if run r1 notap > /dev/null 2>&1; then fail "non-ArduPilot directory accepted"; else ok "non-ArduPilot dir refused"; fi

# 8. bootloader is built once, then reused until the ID or hwdef-bl.dat changes
make_ap a5
mkdir -p "$TMP/a5/Tools/bootloaders" "$TMP/a5/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7"
cat > "$TMP/a5/waf" <<'WAF'
#!/bin/sh
echo "$*" >> "$(dirname "$0")/waf.log"
if [ "$1" = bootloader ]; then
    mkdir -p "$(dirname "$0")/build/LEVIA_H7/bin"
    echo bl > "$(dirname "$0")/build/LEVIA_H7/bin/AP_Bootloader.bin"
fi
WAF
chmod +x "$TMP/a5/waf"
echo "# bl" > "$TMP/a5/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/hwdef-bl.dat"
EB="$TMP/r1/scripts/ensure-bootloader.sh"
sh "$EB" "$TMP/a5" > /dev/null 2>&1 || fail "ensure-bootloader first run failed"
check "bootloader copied" test -s "$TMP/a5/Tools/bootloaders/LEVIA_H7_bl.bin"
check "cleaned before building" grep -qx clean "$TMP/a5/waf.log"
sh "$EB" "$TMP/a5" > /dev/null 2>&1
check "bootloader not rebuilt when unchanged" test "$(grep -c '^bootloader$' "$TMP/a5/waf.log")" = 1
echo "# changed" >> "$TMP/a5/libraries/AP_HAL_ChibiOS/hwdef/LEVIA_H7/hwdef-bl.dat"
sh "$EB" "$TMP/a5" > /dev/null 2>&1
check "bootloader rebuilt after hwdef-bl change" test "$(grep -c '^bootloader$' "$TMP/a5/waf.log")" = 2
make_repo r6 1300
sh "$TMP/r6/scripts/ensure-bootloader.sh" "$TMP/a5" > /dev/null 2>&1
check "bootloader rebuilt after ID change" test "$(grep -c '^bootloader$' "$TMP/a5/waf.log")" = 3

if [ "$fails" -ne 0 ]; then
    echo "$fails check(s) failed" >&2
    exit 1
fi
echo "all install tests passed"
