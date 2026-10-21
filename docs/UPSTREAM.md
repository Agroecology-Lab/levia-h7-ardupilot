# Upstream baseline

The configuration was bench validated against ArduPilot commit:

```text
24b18174d645f85f9ed5b8ce5e0c343072bf6e6a
```

The commit is also stored in `ardupilot/UPSTREAM_COMMIT`, which CI builds.

ArduPilot evolves continuously. Re-run both diagnostics and the complete
bring-up checklist after rebasing this board support onto a newer revision.

The private development build uses provisional ID `AP_HW_LEVIA_H7`. The
number lives in `ardupilot/BOARD_ID` and is added to
`Tools/AP_Bootloader/board_types.txt` by `install-into-ardupilot.sh`, which
refuses to continue if the number is already allocated to another board.

ID `1224` was free at the validated commit but is allocated to
`AP_HW_BrahmaH7` on current ArduPilot master (checked at `e08bc39`), so the
repo now uses `1245`, which was unallocated there. ArduPilot does not provide
a private allocation range: reserve an ID through an upstream change to
`board_types.txt` before firmware distribution.

The bootloader embeds the ID. The prebuilt images in `bootloader/` and
`firmware/` are ID `1224` and match only the validated commit. Anything built
with the current `ardupilot/BOARD_ID` needs its own bootloader, installed
through ST-Link: a bootloader with a different ID cannot upload the
application through the normal USB uploader.

## Hiwonder EMM driver

`ardupilot/patches/` holds four patches (new library, SRV_Channels hook,
arming check, build lists). They apply cleanly to `24b18174` and to master at
`e08bc39`. See `HIWONDER_EMM.md`.
