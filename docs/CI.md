# Continuous integration

## What runs

| Workflow | Trigger | Checks |
| --- | --- | --- |
| `ci` / `static checks` | every push to `main`, every PR | shellcheck, actionlint, `scripts/ci/check_repo.py`, `scripts/ci/test-install.sh` |
| `ci` / `build (copter)`, `build (rover)` | after static checks pass | installs into the pinned ArduPilot commit, builds bootloader and firmware, verifies the image |
| `ci` / `upstream master compatibility` | every push and PR | installs into current ArduPilot master (patches apply, ID free). Informational, does not block |
| `upstream drift` | Mondays 05:17 UTC, manual | full copter and rover build against ArduPilot master. Opens or updates an issue on failure |

Builds use `ardupilot/ardupilot-dev-chibios`, the container ArduPilot's own CI
uses, with its arm-none-eabi GCC 10. The pinned commit is read from
`ardupilot/UPSTREAM_COMMIT`. Move it deliberately and update
`docs/UPSTREAM.md` in the same change, since `check_repo.py` enforces that.

## What a build verifies

- the install script is idempotent (a second run applies nothing)
- the `.apj` board ID equals `ardupilot/BOARD_ID`
- at least 128 KB of flash is free (`MIN_FREE_KB` overrides)
- `AP_HiwonderEMM` is linked into the image
- every line of `defaults.parm` reached the image

Artifacts (`.apj`, `.bin`, the bootloader, `SHA256SUMS`, `build-info.txt`) are
kept for 14 days. They carry a provisional, unregistered board ID, so CI does
not publish releases.

## What CI does not prove

Nothing here runs the firmware. There is no SITL test of the EMM driver and no
hardware test, and compile success says nothing about the I2C register map.

## Suggested branch protection

Require `static checks`, `build (copter)` and `build (rover)`. Leave
`upstream master compatibility` optional.

## Run locally

```sh
shellcheck scripts/*.sh scripts/ci/*.sh
python3 scripts/ci/check_repo.py
sh scripts/ci/test-install.sh
```
