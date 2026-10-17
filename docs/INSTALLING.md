# Installing AmbWave on an FM-1

The browser demo needs no hardware and never installs firmware. AmbWave runs
on the **M-VAVE FM-1**. Use only an AmbWave package from its verified release,
with the matching source and notices. The upstream Felucca installer normally
installs Felucca; it is not an AmbWave download.

## Before installing

1. Back up device projects, presets and samples using Felucca's existing backup
   tools. Keep those private backups outside this repository.
2. Keep the previously working firmware package and the official firmware
   obtained from M-VAVE. AmbWave does not redistribute the official firmware.
3. Download the release bundle. Check its files against SHA256SUMS, for example
   `shasum -a 256 -c SHA256SUMS` on macOS. Extract the matching source archive.
4. Connect the FM-1 directly by USB with stable power. Close other MIDI editors
   and installers before starting. Keep the connection intact until completion.

## Install in your browser

The [AmbWave website](https://mentalcells.github.io/ambwave/) includes an
[Install AmbWave](https://mentalcells.github.io/ambwave/install/) page.
Open it in Chrome or Edge over HTTPS (or localhost for the local preview).
The page checks the exact release package's SHA-256 and compatible identity
before enabling installation. Confirm that you have saved your backup, click
**Install AmbWave**, review the confirmation and allow MIDI SysEx access.
No Python is needed. Loading the page alone never requests MIDI or writes firmware.

Keep USB connected through loader staging, writing, restart and the final
identity check. After completion, verify the AMBWAVE splash and ABOUT version
on the device: FM-1_900 is a compatibility identifier shared with other builds.
The maintainer reports that installation, startup, basic playback, controls and
normal power-cycle restart passed on an FM-1. Rollback and interrupted-update
recovery remain untested on hardware for this release.

The page also accepts a trusted previous Felucca-compatible package for
rollback. Compare the displayed checksum with your saved release first.
Official M-VAVE rollback follows its separate upstream procedure.

## Optional command-line installation from matching source

Use Python 3 in a virtual environment. From the extracted source directory:

```sh
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install mido==1.3.3 python-rtmidi==1.5.8
python3 tools/fm1_install.py --info
python3 tools/fm1_install.py /path/to/ambwave-0.1.0.fwsc
```

The installer asks before writing. It uses the existing Felucca update loader,
then restarts the device and verifies the expected FM-1_900 identity. The USB
MIDI name remains Felucca for compatibility; AmbWave appears on the device.
The identity check alone does not distinguish AmbWave from another compatible
build: use the verified package and check the AMBWAVE splash/ABOUT screen.

Start with TIDE, Shape 50, Motion 25, Density 50 and Space 0. Tap a chord, sustain
it, try the first three knobs, and release the notes. See QUICK_START.md.
Complete AmbWave scene save/recall is not implemented. Existing project storage
must not be treated as a backup of the curated interface and live state.

## Roll back or recover

For a normally running compatible device, run the same installer with your
previously working package. Do not erase user storage as part of rollback.
If interrupted in update mode, reconnect and rerun the installer with the same
verified package; it supports resuming the loader transfer.

If USB no longer exposes a working MIDI update device, stop using the normal
installer and follow the upstream recovery instructions in README_FELUCCA.md
(the section describing WL80UBOOT and FM-1 Transporter). The recovery project
is https://github.com/kurogedelic/FM-1-transporter . Preserve a full flash
backup before recovery writes. Recovery is a separate procedure, not an
automatic part of an AmbWave release or browser visit.
