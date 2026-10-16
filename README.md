# AmbWave

An ambient instrument for the **M-VAVE FM-1**, maintained by **mentalcells**, built on
[Felucca](https://github.com/hugelton/Felucca) by Leo Kuroshita / Hügelton Instruments.
Hold a chord, add atmosphere and scattered notes, then shape the scene with four knobs.

**MVP 0.1.0, release preparation.** The browser and hardware run the same
synthesis code. Source, firmware downloads and a browser demo are being packaged;
no public demo or downloadable release is live yet. AmbWave was previously
called Hasake; historical design notes retain that name.

## Play

- **ALGORITHM** selects one of four tracks.
- **PRESETS** selects its sound: TIDE, TEXTURE, CLOUD, WARM PLUCK, SOFT MALLET,
  BELL PLUCK, TIDE WARM or TIDE SILK. Changing sound clears that track's held notes.
- In **HOLD**, tap notes to sustain them; tap them again to release.
- The four knobs are **SHAPE**, **MOTION**, **DENSITY**, **SPACE**.
- Hold **HOME** and turn the four knobs to mix the tracks. HOME plus the first
  four black keys toggles their mutes. The browser also has visible level sliders
  and mute controls beneath the instrument.
- **FX + OCT−** clears the selected track's held notes.
- **HOME + ARP** cycles HOLD / SCATTER / SEQUENCE. SCATTER uses a held pitch set
  while PLAY runs. SEQ opens the existing step editor for a deliberate phrase.

Start with track 1 on TIDE and tap three notes. Turn SPACE to 0 to hear it dry. Add a quiet TEXTURE
on track 2. On track 3, hold a few notes and press PLAY for sparse plucks.
See [the quick start](docs/QUICK_START.md) for browser controls and mode details.

## What is here

Four independent tracks, curated sounds, a direct mixer and musical macro
mappings. TIDE uses a curated FM6 patch with sustained harmonic movement,
a stable fundamental and no chorus or detuning. TIDE WARM uses the existing analog engine; TIDE SILK uses Felucca's
FM6 engine. CLOUD uses the existing granular engine and a bundled flute sample.
No personal samples are required.

Eight voices and the effects buses are shared. SPACE changes each track's sends
and dry balance. Complete AmbWave scene save/recall is not implemented; underlying
Felucca project storage is not a substitute for restoring the curated interface.
This MVP focuses on making and playing a scene by hand.

## Build and run locally

See [BUILDING.md](BUILDING.md) for dependencies. On the configured development Mac:

```sh
./tools/build-ambwave-macos.sh
./tools/test-macos.sh
AMBIENT_POC=1 sh web/emu/build.sh
python3 -m http.server 8790 --bind 127.0.0.1 --directory build/emu
```

Open `http://127.0.0.1:8790/`, then click **Switch on**. The explicit
`AMBIENT_POC=1` flag is required for the AmbWave browser build; the generic build
retains Felucca's upstream-compatible default. The build creates a `.fwsc`
package but does not install it on hardware.

## Hardware and releases

Do not use upstream Felucca's install link expecting it to install AmbWave.
No public AmbWave installer or release is claimed here yet. Installation and
recovery use the existing Felucca tools. The prepared release site includes a
direct AmbWave browser installer with package checksum verification; read
[installation and recovery](docs/INSTALLING.md) and preserve device backups
before installing experimental firmware.

See [the launch checklist](docs/PUBLIC_RELEASE_CHECKLIST.md),
[installation and recovery](docs/INSTALLING.md), [release notes](CHANGELOG.md),
and [release preparation](docs/MVP_RELEASE.md) for the remaining public-MVP work
and [the design record](docs/HASAKE_DESIGN_DECISIONS.md) for decisions and limits.

## Licence and credits

AmbWave is distributed under **GPL-3.0-only**, with the original copyright and
third-party notices retained. Read [LICENSE](LICENSE) and [LICENSING.md](LICENSING.md)
for the complete component credits and licences. Distribute the matching source
and licence material with releases; do not treat this README as the full notice.

Felucca provides the synthesis engines and device infrastructure. The browser
emulator follows work by [Charles Vestal / X0X](https://github.com/charlesvestal/fm1-x0x).
Font, sample and DSP credits are listed in LICENSING.md. The original upstream
README is preserved as [README_FELUCCA.md](README_FELUCCA.md).

Report AmbWave issues in [this repository](https://github.com/mentalcells/ambwave/issues).
AmbWave is independent of M-VAVE and Hügelton Instruments; it is not endorsed by
those projects or owners. M-VAVE and FM-1 are trademarks of their respective owners.
