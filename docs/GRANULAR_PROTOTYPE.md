# CLOUD granular prototype

2026-10-13. Branch `codex/ambient-granular`, based on the committed
three-layer prototype `2c9a25a` (descended from `ambient-prototype`).

## Playing

Boot remains TIDE + TEXTURE + PULSE. Select layer 2 with ALGORITHM and turn
PRESETS on HOME to alternate TEXTURE and CLOUD. A sound change releases the
layer's held note and starts silent; tap a key to hold the new sound, tap it
again to release. CLOUD holds one physical pitch, using the same layer-2 MIX,
HOME mute, clear-notes and external MIDI ownership paths. Other layers keep
playing. HOME preset selection preserves layer level and shared FX settings.
Returning to TEXTURE restores its curated macro defaults.

No user samples are needed. CLOUD uses Felucca's built-in FLUTE source (the
same source used by its CLOUD PAD preset), with forward grains and no random
pitch detuning. Piano and sax material are also available for later audition.
There is no new DSP or sample data in this prototype.

## Macros

- SHAPE: SIZE 48–112, TONE 112–88, from shorter fragments to longer/darker grains.
- MOTION: existing DRIFT LFO scans position with depth 0–38; source-position
  spread 0–55. No pitch modulation. Zero is a frozen source position.
- DENSITY: DENS = 42 + 0.70 × density − 0.18 × shape, to reduce the rate as
  grains grow longer. This is approximate overlap compensation, not exact
  independence; the existing grain pool/load cap still applies.
- SPACE: existing TEXTURE local delay/reverb sends and dry perspective curve.
  TIDE continues to own global effects parameters.

Defaults: shape 35, motion 25, density 50, space 50. Mix level is retained.
Detailed engine editing remains available through Felucca's pages.

## Validation and limits

The baseline unmodified-upstream build/test evidence is in AMBIENT_PLAN.md.
Build using `tools/build-hasake-macos.sh` (invokes `./build.sh` with
AMBIENT_POC=1); run `tools/test-macos.sh` (invokes `sh tests/run_tests.sh`).

Focused granular tests cover HOME selection, silent switching, held-note
release, level/global-FX preservation, all 16 macro corners alone and with
four-note TIDE, and return to curated TEXTURE. HOME/MIX render lint includes
CLOUD across themes/styles/sizes/mutes. The full suite retains engine CPU,
audio hash regression and storage compatibility checks; no golden hashes change.

Listening previews are in `build/granular_demo/`: `cloud-default.wav`,
`cloud-sparse.wav`, `cloud-full.wav`, and `tide-cloud.wav` (host renders).

Validation completed: focused CLOUD checks passed; 7,680 HOME/MIX screens
had zero lint/alignment findings. Full host checks passed their substantive
checks, including 92 unchanged golden renders, zero health/voice failures,
and zero CPU entries over budget. The suite's WebAssembly build initially
failed on sandbox access to Emscripten's compiler cache; rebuilding with cache
access and rerunning the browser/native comparison passed. The optional DaisySP
reference test was skipped because its checkout is absent.

Firmware package builds: image 438812 bytes, static RAM 89788/98304 bytes,
pool 330208/344064 bytes. No hardware installation was performed.

Listening and on-device CPU margin remain unverified. This is a selectable
alternative on layer 2, not a fourth simultaneous layer. External MIDI can
still consume shared voices. Macro identities and CLOUD selection are runtime
state; scene persistence is not added. Source changes are deliberately silent,
rather than attempting to transfer a held pitch across different engines.
