# Hardware rollback: playing modes before CLOUD bloom

This is the exact package installed on 2026-10-14 and subsequently enjoyed on
the user's FM-1, before CLOUD HOLD/SCATTER tuning. Firmware source: `4c42c71`;
installation record: `0e47fbc`.

SHA-256: `4018e1c51638423740af3010eac3adc367d7abb690a5ac435ea366cc5657bb95`.

Git rollback tag: `codex/playing-modes-before-cloud-bloom`. The tag contains the
package and the unchanged firmware source. This package is deliberately tracked
as a small rollback artifact despite the general `*.fwsc` ignore rule.

Restoring hardware requires explicit approval in the conversation performing
the restore. Run the existing installer against the package; do not use --force.
Neither checking out the tag nor building firmware installs it on the device.

```sh
.venv/bin/python tools/fm1_install.py artifacts/rollback/playing-modes-4c42c71/hasake-playing-modes-4c42c71.fwsc
```

## Clean source repository

The historical rollback binary is retained only in the private development
archive and local checkout. It is not distributed in this source snapshot.
The paths and old commit identifiers above describe that earlier local work.
