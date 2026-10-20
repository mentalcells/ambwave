# Autonomous Drone MVP

Implemented 2026-10-20 on `codex/autonomous-drone`, continuing the existing
uncommitted factory-audition work. Software evaluation; listening acceptance
and actual FM-1 timing remain pending. Implementation validation did not flash
hardware; the subsequent user-authorized installation is recorded below.

## Performance workflow

- Select any of the four tracks with ALGORITHM.
- Hold HOME and turn PRESET to DRONE (one step backwards from TIDE).
- The single sound is AN SOFT PAD, a role profile over the existing ANALOG
  factory SOFT PAD. The factory bank and Tide's factory audition are unchanged.
- Press a note or overlapping chord of up to four notes. Release all keys;
  the notes keep sounding. No PLAY command is needed.
- After all keys are released, the next press replaces the old harmony.
  Overlapping presses build the new chord. Repeating the same note starts a
  new gesture; it does not toggle the Drone off. Rolling fully separated notes
  replaces the harmony rather than adding notes.
- Select and play another track; the Drone continues evolving.
- HOME + the first four black keys mute/unmute tracks, preserving Drone state.
- **FX + OCT− clears the selected track's Drone.** Existing release and spatial
  tails decay normally. STOP controls generated notes and leaves Drone running.
- Sound/role changes clear live input through the existing sound-change path.
  Drone always uses sustained HOLD; HOME + ARP reports DRONE IS SUSTAINED.

MIDI uses the same gesture grouping and four-note cap, retaining existing
channel/selected-track ownership. A note-off still belongs to the track where
its note-on occurred. MIDI pedal-owned voices retain existing sustain behavior:
old pedal-held notes may overlap a replacement until pedal-up. CC123 clears
when no physical/MIDI input is held; CC120 uses the existing all-sound-off path.
USB and TRS share existing ownership semantics. Only locally emitted latched
notes send output note-offs on replacement/clear; MIDI-only input is not
echoed or followed by unmatched output note-offs. No MIDI clock is required.

## Sound and macros

The source is the existing two-oscillator ANALOG SOFT PAD, with its factory
sustain, attack, release, filter envelope and spatial sends. Drone changes are
separate from the source preset. Density's midpoint uses approximately 18 cents
of oscillator separation, close to the factory's 20 cents.

| Macro | Mapping |
| --- | --- |
| SHAPE | Smooth filter offset, ±24 parameter steps around the source cutoff |
| MOTION | Depth of three independent random walks; higher settings also shorten subsequent target journeys |
| DENSITY | Smooth oscillator separation, approximately 0–36 cents; no additional voices |
| SPACE | Existing chorus/delay/reverb sends; 50 restores source sends, 0 removes all three, higher values increase sends and reduce dry gain |

Defaults: SHAPE 50, MOTION 25, DENSITY 50, SPACE 50. Motion zero settles to
static modulation; factory beating and spatial effects can still move.

Three independent, deterministic per-track random streams of targets (one
private RNG per track) drive cubic smooth interpolation. Individual target
journeys range from 20 to 120 seconds across settings, with unequal durations.
At the default, the maximum journey is approximately 103 seconds; at maximum
Motion it is 50 seconds. Endpoints have near-zero slope. Timbral movement is
bounded to ±12 cutoff steps, breathing attenuates by at most approximately
12.5%, and first-oscillator drift stays within approximately ±1.5 cents.
The other oscillator maintains the harmonic foundation. Notes/key/scale never
change autonomously. Macro values are smoothed at block rate; existing gain
ramps handle breathing between blocks. No per-sample random/LFO calculation,
new audio buffer, effects algorithm, engine, or modulation-matrix UI is added.

## Limits

Eight voices remain shared by all tracks. The four-note latch cap does not
reserve four voices. Replacement uses old release tails and new normal attacks;
no extra crossfade voices are reserved. Dense chords and simultaneous tails
can steal voices through the existing bounded fade path. Seamless transitions
under exhausted polyphony are not guaranteed. The recommended scene uses a
Drone fifth, a Tide dyad, one Texture note and sparse single Pulse events.

Drone identity and evolution state are runtime-only, matching existing ambient
metadata. Projects keep their existing format and save engine parameters;
complete ambient-role/scene recall remains outside this milestone. Evolution
is applied during rendering and does not overwrite stored patch parameters.

## Audition

Local browser preview: <http://127.0.0.1:8795/>. Click Switch on; Shift-click
HOME to hold/release it. Hold HOME and turn PRESET one step backwards from the
initial TIDE role, then release HOME. Play an overlapping low note/fifth and
release it. Try Motion at 0, 25 and 100, leaving each running for several minutes.
Use ALGORITHM to play another track; use the mixer to mute/unmute the Drone.
Use FX + OCT− to clear it.

Restart the preview if necessary:

```sh
AMBIENT_POC=1 sh web/emu/build.sh
python3 -m http.server 8795 --bind 127.0.0.1 --directory build/emu
```

Repeatable five-minute PCM recordings are generated by:

```sh
mkdir -p build/drone
cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src \
  -o build/drone/audition tools/drone-audition.c -lm
build/drone/audition build/drone/solo-motion0.wav 0 solo
build/drone/audition build/drone/solo-motion25.wav 25 solo
build/drone/audition build/drone/solo-motion100.wav 100 solo
build/drone/audition build/drone/four-track-scene.wav 25 scene
```

The scene is Drone C2/G2, Tide C4/G4, quiet Texture C3, and sparse Pulse C5
using BELL PLUCK. It starts once and evolves without further input. Recordings
are deterministic within the same build. They have been rendered and measured,
not validated by listening. Audition for subtle life, repetitive movement,
brightness/loudness balance, usefulness under Tide, and natural intervention.

Hardware evaluation, following separately authorized installation: repeat the
browser workflow, test USB/TRS MIDI chords and track switching, check replacement
under dense four-track use, and listen for overload/dropouts and transition
clicks. Building the package does not authorize installation.

## Validation and resources

- Final `tools/test-macos.sh` / `sh tests/run_tests.sh`: **ALL HOST TESTS PASSED**.
  All 92 golden renders unchanged; zero health, voice/routing, or host CPU-budget
  failures. Existing storage compatibility, full UI, target instruction guards,
  simulated installer and native/WebAssembly emulator checks pass. The optional
  DaisySP float-reference comparison was skipped because its checkout is absent.
  Full output: `build/drone/host-suite-results.txt`.

- Firmware: `tools/build-ambwave-macos.sh` (invokes `./build.sh`) passed.
  Image 443700 bytes (1316 above prior factory-audition build); static RAM
  90368/98304 bytes (420 more; 7936 free). Pool 330208/344064 bytes, unchanged
  (13856 free). Private Drone evolution state is 384 bytes for all four tracks;
  latch input bookkeeping and metadata account for the remaining RAM increase.
- Package: `build/felucca.fwsc`, 609849 bytes. SHA-256:
  `ffcedb875412e076157d7482b7be751f471ed83c42fcd44ec8fb6ac061f38ec3`.
- Focused Drone tests cover all four track positions, physical/MIDI latching,
  replacement, selection ownership, mute, actual clear gesture, CC120/123,
  mode lock, sound changes, one-sound browsing, undo/redo and existing project
  serialization. Five-minute control runs cover Motion 0/25/50/75/100;
  parameter storage is unchanged throughout. Signed-overflow and divide-by-zero
  sanitizer checks of the focused test also pass.
- Browser emulator compiled. Drone's public panel-gesture sequence produces
  **2890314 identical stereo frames** in native C and WebAssembly, including a
  minute without note events, track switching, replacement and all macros.
  Existing ambient emulator boot/save/mixer/audio/cost checks also pass.
- HOME/mixer UI: **614400 layouts**, zero lint or alignment findings. Drone
  HOME and mixer screenshots were visually inspected. Existing CLEAR hint is
  visible alongside the role and sound name.
- Target static instruction guard passes with existing budgets unchanged:
  ISR weighted loop cost 40292 against baseline 37502 (+7.4%, within the
  existing +10% limit). Drone's block-rate helper is kept out of line and
  separately guarded at cost 599; its note-event helper at 21. These are static
  estimates, not measured hardware cycles. New optional helper budgets are
  recorded in `tests/target_budget.py`; no old budget or golden hash was raised.
- Host costs are measured on this Mac, not on the FM-1. Four active single-note
  Drone tracks cost approximately 0.0015 of real time. The mixed five-minute
  scene costs approximately 0.0019. Target timing and musical acceptance remain
  open.

| Five-minute recording | Peak (16-bit) | RMS | Mean/DC | Clipped samples |
| --- | ---: | ---: | ---: | ---: |
| Solo Motion 0 | 12343 | 3158.58 | -2.10 | 0 |
| Solo Motion 25 | 12339 | 3094.56 | -2.00 | 0 |
| Solo Motion 100 | 12071 | 2933.42 | -1.58 | 0 |
| Four-track scene Motion 25 | 8631 | 2074.32 | -0.29 | 0 |

Every recording retains both gated Drone notes at five minutes. These
measurements establish numerical bounds, not richness, transition quality or
musical interest. Listening is still needed.

Manual browser equivalence reproduction (after compiling the ambient wasm):

```sh
cc -O2 -ffp-contract=off -w -DAMBIENT_POC=1 -Ibuild/gen -Ifirmware/src \
  -o build/drone/native-check web/emu/native_check.c -lm
build/drone/native-check build/drone/native.f32 drone
node tests/drone_web_check.mjs build/emu/felucca.wasm build/drone/native.f32
```

The preview wasm snapshot is `build/emu/drone-preview/felucca.wasm`, SHA-256
`38bd20fce6d703e6b114aac807321ceedff61d1415e6e2e80df4a47be0b3cbe2`.
The normal host suite also builds a generic Felucca emulator into `build/emu`;
use the separate Drone preview directory or rebuild with `AMBIENT_POC=1`
afterward.


## Hardware installation — 2026-10-20

The user explicitly requested flashing this Drone build in the current
conversation. The verified package above was installed through the unchanged
`tools/fm1_install.py` USB-MIDI updater on the connected Felucca port. Package
checks, loader transition, 100% write, restart and final FM-1_900 identity
verification completed successfully (exit 0). Log:
`build/drone/hardware-install-results.txt`.

This confirms updater completion and identity response, not listening quality,
visual splash confirmation, or actual target timing. Hardware audition remains
pending. No user-storage erase or preset/sample/project write was requested.
