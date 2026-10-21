#!/usr/bin/env python3
"""Static repository checks. No network, no ArduPilot checkout needed."""

import hashlib
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
errors = []


def err(msg):
    errors.append(msg)


def read(rel):
    return (ROOT / rel).read_text()


def tracked():
    out = subprocess.run(["git", "ls-files", "-s"], cwd=ROOT, check=True,
                         capture_output=True, text=True).stdout
    for line in out.splitlines():
        meta, path = line.split("\t", 1)
        yield meta.split()[0], path


# board ID: single source of truth, outside the ODID range
board_id = read("ardupilot/BOARD_ID").strip()
if not board_id.isdigit():
    err("ardupilot/BOARD_ID must be a number")
elif not 1 <= int(board_id) < 10000:
    err("ardupilot/BOARD_ID must be below 10000 (ArduPilot reserves higher IDs)")

# pinned upstream commit must be a full hash and documented
commit = read("ardupilot/UPSTREAM_COMMIT").strip()
if not re.fullmatch(r"[0-9a-f]{40}", commit):
    err("ardupilot/UPSTREAM_COMMIT must be a full 40 character hash")
elif commit not in read("docs/UPSTREAM.md"):
    err("docs/UPSTREAM.md does not mention ardupilot/UPSTREAM_COMMIT")

# scripts: shebang and executable bit
for mode, path in tracked():
    if path.endswith(".sh"):
        if mode != "100755":
            err(f"{path}: not executable in git")
        if not read(path).startswith("#!"):
            err(f"{path}: missing shebang")

# text files must not contain CRLF
TEXT_SUFFIXES = {".sh", ".md", ".dat", ".parm", ".py", ".yml", ".yaml",
                 ".cpp", ".patch", ".txt"}
for _, path in tracked():
    p = Path(path)
    if p.suffix in TEXT_SUFFIXES or p.name == "wscript":
        if p.parts[0] == "firmware":
            continue
        if b"\r\n" in (ROOT / path).read_bytes():
            err(f"{path}: CRLF line endings")

# vendored patches: numbered, contiguous, real diffs, no stale board patch
patch_dir = ROOT / "ardupilot" / "patches"
patches = sorted(patch_dir.glob("*.patch"))
for i, patch in enumerate(patches, start=1):
    if not re.fullmatch(rf"{i:04d}-.+\.patch", patch.name):
        err(f"{patch.name}: expected sequence number {i:04d}")
    if "diff --git" not in patch.read_text():
        err(f"{patch.name}: contains no diff")
if (ROOT / "ardupilot" / "board_types.patch").exists():
    err("ardupilot/board_types.patch must not exist, install script inserts the ID")

# hwdef consistency
for name in ("hwdef.dat", "hwdef-bl.dat"):
    text = read(f"ardupilot/hwdef/LEVIA_H7/{name}")
    if not re.search(r"^APJ_BOARD_ID\s+AP_HW_LEVIA_H7\s*$", text, re.M):
        err(f"{name}: APJ_BOARD_ID must be AP_HW_LEVIA_H7")
if any("HiwonderEMM" in p.name for p in patches):
    if not re.search(r"^define AP_HIWONDEREMM_ENABLED 1\s*$",
                     read("ardupilot/hwdef/LEVIA_H7/hwdef.dat"), re.M):
        err("hwdef.dat: AP_HIWONDEREMM_ENABLED not defined but driver patches exist")
if not (ROOT / "ardupilot/hwdef/LEVIA_H7/defaults.parm").exists():
    err("ardupilot/hwdef/LEVIA_H7/defaults.parm missing (install script copies it)")

# validated firmware images must match the checksums in firmware/README.md
readme = read("firmware/README.md")
listed = re.findall(r"^([0-9a-f]{64})  (\S+)$", readme, re.M)
if not listed:
    err("firmware/README.md lists no checksums")
for digest, name in listed:
    matches = list((ROOT / "firmware").rglob(name))
    if not matches:
        err(f"firmware/README.md lists {name} but no such file exists")
        continue
    for m in matches:
        actual = hashlib.sha256(m.read_bytes()).hexdigest()
        if actual != digest:
            err(f"{m.relative_to(ROOT)}: sha256 mismatch ({actual})")

# relative markdown links must resolve
LINK = re.compile(r"\]\(([^)\s#]+)(?:#[^)]*)?\)")
for _, path in tracked():
    if not path.endswith(".md"):
        continue
    for target in LINK.findall(read(path)):
        if re.match(r"[a-z][a-z0-9+.-]*:", target):
            continue
        if not (ROOT / path).parent.joinpath(target).resolve().exists():
            err(f"{path}: broken link {target}")

if errors:
    print("repository checks failed:", file=sys.stderr)
    for e in errors:
        print(f"  - {e}", file=sys.stderr)
    sys.exit(1)
print("repository checks passed")
