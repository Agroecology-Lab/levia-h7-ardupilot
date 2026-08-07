# Upstream baseline

The configuration was bench validated against ArduPilot commit:

```text
24b18174d645f85f9ed5b8ce5e0c343072bf6e6a
```

ArduPilot evolves continuously. Re-run both diagnostics and the complete
bring-up checklist after rebasing this board support onto a newer revision.

The private development build uses provisional ID `AP_HW_LEVIA_H7` (`1224`).
This number was unused in the official registry when selected, but ArduPilot
does not provide a private allocation range. Reserve it through an upstream
change to `Tools/AP_Bootloader/board_types.txt` before firmware distribution.

The bootloader and ArduCopter images have both been build-verified with ID
1224. Hardware validation still requires deliberately installing the new
bootloader through ST-Link; an existing ID-1013 bootloader cannot upload an
ID-1224 application through the normal USB uploader.
