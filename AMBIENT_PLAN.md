# AmbWave

Renamed from Hasake on 2026-10-16. The user accepted the installed FM TIDE
as the MVP direction. Current launch work: [public checklist](docs/PUBLIC_RELEASE_CHECKLIST.md).
Historical milestone notes retain their original terminology.

## Intent

Turn the M-VAVE FM-1 into an immediate ambient instrument built on Felucca.
The experience should emphasize evolving sound, playable transformation, and
minimal menu navigation rather than sequencing depth or feature count.

## Design record and current status

Read `docs/HASAKE_DESIGN_DECISIONS.md` for accepted sound/layer/scene terminology,
hardware feedback, limits and the proposed next order. TIDE + TEXTURE layering
and the direct mixer have now received positive hardware listening feedback.
The user accepted a clocked third layer as the next product milestone, before
scene saving; see `docs/ARP_LAYER_PROTOTYPE_SPEC.md`. The third PULSE layer is now implemented for software evaluation; see
`docs/ARP_LAYER_PROTOTYPE_RESULTS.md` for arp audit, mappings and evidence.
Initial PULSE listening found poor sound/pacing and a HOME preset-loss bug.
The user chose scattered random plucks from held notes; the follow-up fix and
three HOME timbres are described in `docs/PULSE_PRESET_FIX_RESULTS.md`.
The user then approved stronger PULSE macros, independent speed/density and
HOME layer mutes; see `docs/PULSE_CONTROLS_RESULTS.md` for controls and evidence.
Revised sound tuning remains under evaluation. Upstream integration remains separate work; scene persistence
follows a clearer playing model.

The independent sound-slot prototype now boots TIDE / TEXTURE / PULSE / CLOUD
on four layers. Any layer can select any curated sound while retaining HOME;
see `docs/SOUND_SLOTS_RESULTS.md` for controls, validation and shared-resource
limits. It supersedes the fixed layer-2 CLOUD/TEXTURE selection.

A further playing-mode experiment on `codex/ambient-playing-modes` separates
HOLD / SCATTER / SEQUENCE from the chosen sound. CLOUD can now play an entered
phrase through the existing step editor; see `docs/PLAYING_MODES_RESULTS.md`.
Implementation `4c42c71` was installed on the FM-1 on 2026-10-14 with explicit
user authorization. The user subsequently found the firmware fun, including
CLOUD SCATTER. The reversible CLOUD hold/length experiment is recorded in
`docs/CLOUD_BLOOM_RESULTS.md`. The next sound priority is TIDE warmth and body,
using patch comparisons before adding synthesis engines.
The first WARM PAD (ANALOG) and SILK PAD (existing FM6) candidates are available
alongside original TIDE on `codex/warm-pads`; see `docs/WARM_PADS_RESULTS.md`.
After positive emulator feedback on their basic tone, the user requested more
audible controls and retained TIDE naming for the MVP. That follow-up is on
`codex/tide-performance-macros`; see `docs/TIDE_MACROS_RESULTS.md`. Public release
preparation is recorded in `docs/MVP_RELEASE.md`; visibility/hosting have not
been changed.
The dry-source experiment on `codex/tide-dry-source` tests sustained saw/triangle
harmonic evolution in original TIDE and makes SPACE=0 disable its chorus send.
See `docs/TIDE_DRY_SOURCE_RESULTS.md` for level-matched dry A/B audio and routing
evidence. User listening found it less harsh and more like EQ, but not
necessarily richer; this candidate has not met the richness criterion.
The replacement experiment, `docs/TIDE_HARMONICS_RESULTS.md`, tests independent
low/middle harmonic exchanges around a steady fundamental. It preserves the
four macros and uses separate dry A/B recordings. User listening found it
“Different, but not clearly richer”; this candidate also misses the richness goal.
The user then approved the fullness of both isolated AMB-1 dry reference pads,
with Rhodes feeling more grounded. `docs/TIDE_FM_PATCH_AUDITION.md` records a
listening-only patch test using Felucca's existing FM6 engine and matched notes.
The user heard the FM patch as “It gets closer”; retain it as the strongest
direction so far. After similar-sounding midrange variants, the user approved
integration of the original FM candidate into TIDE. The default dry render is
bit-identical to that audition; Shape controls FM complexity, Motion its drift,
and Density harmonic weight, with Space=0 disabling spatial sends. A playable
emulator is prepared; final macro listening acceptance remains pending.

## First playable idea

Powering on should lead quickly to a curated ambient sound that can sustain or
latch. The four main knobs should behave as musical macros:

- **SHAPE** — moves the sound from clear and tonal toward smeared or complex.
- **MOTION** — adds slow modulation and organic drift.
- **DENSITY** — increases layers, grains, detuning, or harmonic weight.
- **SPACE** — increases delay/reverb and adjusts the dry/wet balance musically.

The mapping may differ by engine, but the four concepts should remain stable
for the player.

## Milestones

### 0. Development baseline

- [x] Build unmodified Felucca 1.0.5.2 on macOS.
- [x] Run its host test suite (`ALL HOST TESTS PASSED`).
- [x] Record the exact toolchain and SDK versions.
- [x] Do not flash hardware in this milestone.

Baseline environment established on 2026-10-07:

- macOS 15.7.3 on Apple Silicon (`arm64`)
- Python 3.14.6 with the packages in `requirements-dev.txt`
- Docker Engine 29.8.2
- JieLi toolchain `jieli-linux-toolchains-20250324.1`
- AC79 SDK tag `AC79NN_SDK_V1.2.1_2023-12-13`, commit
  `d179b4484759423312073f5fbb232501aa491047`
- Baseline package SHA-256:
  `1bf6622531c9d57fd96949568d13c94d30414c6ac26517e9fc20e36dbc8c4444`

On this Mac, build with `tools/build-macos.sh` and test with
`tools/test-macos.sh`. Dependencies live in ignored `.venv/` and
`.toolchains/` directories.

AmbWave hardware packages must be built with `tools/build-ambwave-macos.sh`. It
pins `AMBIENT_POC=1`; the generic wrapper intentionally retains Felucca's
upstream-compatible default.

### 1. Harmless identity build

- [x] Give the fork a temporary on-screen development name (`HASAKE`).
- [x] Build and test the package.
- [x] Verify the browser emulator before considering a hardware install.

Completed locally on 2026-10-08. The device and browser splash show `HASAKE` /
`AMBIENT OS DEV`; protocol, USB, storage and package identity remain Felucca-compatible.
The full host suite, browser emulator and native/WebAssembly audio comparison passed.

### 2. One ambient voice

- Start from one existing Felucca engine, initially GRAIN or ANALOG.
- Add a curated preset with a safe sustained/latching performance flow.
- Map SHAPE, MOTION, DENSITY, and SPACE to existing parameters.
- Keep SPACE as one HOME performance macro with deeper component editing available
  through Felucca's existing FX pages. Its curve should move from close/dry to a
  clearly audible, stable wash rather than merely exposing a linear reverb amount.
- Keep MOTION as one HOME performance macro over the existing track LFO. Use
  smooth, irregular DRIFT movement of the ANALOG filter: the macro raises both
  depth and activity, 0% is still, and pitch remains stable. Hardware listening
  found the original slow sine too obviously cyclic.
- Keep DENSITY as one HOME macro over ANALOG oscillator mix, detune and chorus.
  Boot at 50% to preserve TIDE; 0% uses one oscillator without detune/chorus,
  and 100% widens the equal oscillator blend with 36-cent detune and chorus send
  100. Keep polyphony and the four-note latch unchanged; no unison.
- Keep sequencing, storage, USB, MIDI, and recovery behavior unchanged.

### 3. Small instrument set

- Add a few deliberately different ambient voices rather than a large bank.
- Consider GRAIN for texture, ANALOG for warmth, and FM6/PHYS for harmonic
  contrast only after the first voice is convincing.
- Evaluate user-sample import after the built-in-source workflow is stable.

The two-layer TIDE + NOISE TEXTURE prototype is implemented for evaluation;
see `docs/TEXTURE_PROTOTYPE_RESULTS.md` for controls, software evidence and
shared-FX/MIDI voice-pressure limits. The user manually installed the package and confirmed that TIDE + TEXTURE works
well and mixing is useful. Focused edge-case listening remains before expansion.

## Out of scope for the first prototype

- A new synthesis engine.
- A rewritten bootloader, loader, or installer.
- A large preset library.
- A new sequencer or groovebox workflow.
- Automatic installation on a physical FM-1.

## Definition of a successful first prototype

Within thirty seconds of power-on, a player can hold or latch an evolving sound
and make four clearly audible, musically useful transformations without menu
diving.
