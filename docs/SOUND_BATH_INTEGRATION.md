# Sound Bath reuse experiment

2026-10-20. Integrated development source; listening and hardware timing
acceptance remain pending. The independent Sound Bath source was not modified.

## Baseline and provenance

The integration builds on AmbWave v0.1.0 plus the autonomous Drone, role/sound
separation and factory-audition changes. It preserves existing track scheduling,
voice management, storage formats and effects algorithms. Local experimental
history and dependency links are excluded from the public commit.

Implementation reference: the independent Sound Bath project, branch
`sound-bath/audition`, commit `0a628cccf50ed429725c55e019029997f20ea85d`.
Its Felucca foundation is v1.6.1 `a53ecf428c29c780108fd70a31b71a32a6b5c313`.
Preserved GPL-3.0-only notices on reused adapters and application code; inherited
Felucca/msfa/DaisySP/Rings credits and notices remain in place. No external
patch bank or new synthesis code was imported.

## Implementation audit

| Source / functions | Existing responsibility | Reuse in AmbWave |
| --- | --- | --- |
| `eng_phys.c`: `PHYS_PRESETS[3]`, `phys_note_on`, `phys_render` | BOWED METAL record, modal impulse, retrigger, continuous random excitation via P_E6; track ADSR controls amplitude and release | Exact existing record; no PHYS code changes |
| `phys_dsp.c`: modal block/run; `phys_symp.c` | Resonator and exciter DSP | Files are identical between inspected repositories; retain AmbWave copies |
| `tools/gen_fm6_patches.py`, FM6 factory slot 1, `fm6_core.c` | GLASS BELL operator patch, algorithms, envelopes, note termination | Generator identical; existing PULSE GLASS BELL already selects that record |
| `sound_bath.c`: `bath_patch_audio` | Loads factory records; BOWL STUDY overrides attack=0, release=100, initial P_E6=0; removes spatial sends | Adapt only initialization in `ambient_patch_apply`; factory table untouched |
| `sound_bath.c`: `bath_audio` | Per-block integer approach to Tone/Excite/Space/Level; key edges, latch toggle and note-off bookkeeping | Adapt parameter approach only; leave track scheduling, voice management and input ownership to AmbWave |
| `sound_bath.c`: `bath_release_audio` / clear path | All notes off; clears key ownership. Explicit clear also zeroes every voice and reverb | Use AmbWave's per-track `ambient_latch_clear`, existing note-off and panic paths; do not import global clearing into local track controls |
| `tools/audition/engine.c`: `sbb_init`, `approach`, `sbb_render` | Original WAV scenarios, including two-block control cadence | Reuse adapter/scenarios in `tools/sound-bath`; retain two-block cadence in integrated control hook |
| `tools/audition/verify.mjs` | Native/WASM pressure, caps, release, fresh-instance silence | Adapt to selected patches 0/3/5 and dry routing |
| `tests/sound_bath_fm1_test.c`, `tools/fm1` | Source UI hold/strike, release, clear, octave and display parity evidence | Inspect as source contract; use AmbWave UI and its own full-panel native/WASM tests instead of importing Sound Bath UI |

Sound Bath does not implement a new strike-to-sustain synthesizer: PHYS already
injects strikes and continuous random excitation into the same modal body.
BOWED METAL starts at excitation 90. BOWL STUDY starts at 0, preserving a struck
onset; raising excitation feeds that existing resonator continuously. It retains
all other preset parameters: `{MODAL,98,70,112,10,80,90,0}`, with P_E6 replaced
by 0 only in the labeled variation. Sustaining a decaying FM bell does not turn
it into an infinite source; its existing operator envelopes remain authoritative.

Sound Bath has two smoothing cadences: FM1 `bath_audio` updates every block;
WAV/browser audition `sbb_render` updates every two CTL blocks. This experiment
matches the recorded audition cadence (one integer step per two blocks), without
adding a new exciter, hold envelope, voice manager, navigation or display.

## Engine revisions checked before importing

PHYS wrapper differences in Sound Bath are static array versus engine-memory
claims (`ENG_MEM`, `ENG_MEM_FITS`, `.mem`) and an optional CPU2 per-track seed.
Single-core synthesis and preset records match. AmbWave retains its original
memory pool and single-core engine host.

Newer FM6 adds bank/import support, engine-memory allocation and optional CPU2
local render buses. AmbWave has its existing opt-in Tide experiments and
carrier-body runtime field. Those features are inactive for PULSE GLASS BELL.
There is no wholesale engine-host or bank/storage upgrade. The verified patch
and scenario equivalence below is the evidence for compatibility, rather than
an assumption that the complete hosts are identical.

A subtle dependency matters: the neutral track LFO clock must stay at its
source default even with zero modulation depth. LFO wraps consume RNG; changing
that clock changes fresh PHYS note seeds. The first long note still matched,
but the next BOWED METAL interaction phrase initially differed. Preserving the
source clock at Motion=0 restored exact audio for the whole recording.

## AmbWave adaptation and controls

Append TIDE IDs **8 BOWED METAL**, **9 BOWL STUDY**; keep all existing IDs.
PULSE ID **3 GLASS BELL** already supplies the selected bell. PRESET selects
sounds; HOME + PRESET selects role; ALGORITHM selects track. Existing HOLD,
SCATTER, SEQUENCE, MIDI ownership, mixer, track muting and eight-voice allocation
remain in charge. PHYS caps at three voices per track; FM6 at six, sharing eight.
A fourth TIDE input can therefore steal a resonator voice. This is inherited
AmbWave/Felucca allocation, not a new Sound Bath voice policy.

| Macro | PHYS adaptation | Default |
| --- | --- | --- |
| SHAPE | Source Tone offset -20..+20 around brightness 70, same integer approach | 50 = unchanged |
| MOTION | Existing AmbWave DRIFT LFO, shallow brightness modulation, depth 0..8 | 0 = source clock and no audible modulation |
| DENSITY | Source EXCITE target 0..100, slewed; no ADSR rewrite | BOWED 90, BOWL 0 |
| SPACE | Source reverb-only send 0..64, slewed, unity dry gain | 0 = dry |

The new Motion mapping is an explicit AmbWave performance extension, separate
from neutral source equivalence. PULSE retains its established factory-audition
macro behavior: Density controls probability; Space controls existing sends;
Shape/Motion remain FACTORY TONE, preserving the accepted bell patch. No new
FM macro design is implied by the source reuse request.

Patch/role changes clear local held input through the existing transition
contract. AmbWave HOLD toggles owned notes; releasing physical keys preserves
the latch. Clear releases resonators naturally; shared FX tails follow AmbWave's
existing effects behavior. It does not copy Sound Bath's REC global hard reset.
Project format is unchanged. As before, curated role/sound identities and
complete scene recall are not serialized; this experiment does not add saving.

## Equivalence and measurements

`SOUND_BATH_EQUIVALENCE.json` records all **16 available matching dry WAVs**:
five GLASS BELL, four BOWED METAL, seven BOWL STUDY. Every WAV is byte-identical
to the source recording, including headers and all signed 16-bit stereo samples;
max difference=0, difference RMS=0. The reused source scenarios cover C3/C4/C5
attack and decay, 60-second sustain, 90-second bowl excitation/release, repeated
strikes, intervals, control sweeps and pressure as available in the source corpus.
The verifier requires equality; it does not update upstream goldens.

Native/WASM pressure audio is sample-exact for all three selected sounds. Caps
are six FM / three PHYS; release and fresh-instance silence pass. Focused AmbWave
checks cover all tracks, source records, mode changes, latch/unlatch, macro
endpoints, sound changes, all 16 macro corners in a four-track scene, stealing
and draining all voices after clear. Stress scenes reached eight active voices,
peaks stayed below full scale, and DC remained bounded at the tested faders.

A repeatable 90-second scene uses two BOWL STUDY resonances and two sparse
PULSE GLASS BELL pitches through the real arp, mixer and FX. Peak=6085/32768,
RMS=421.42 at conservative track levels. Source excitement rises at 5–25s,
Shape changes at 35s, Motion at 45s, Space at 55s, and notes release at 70s.
This demonstrates a controlled audition route; no subjective quality acceptance
or target-hardware timing is claimed.

Wet output is deliberately not called equivalent: Sound Bath uses the newer
HALL (type 2, size 70, damping 80); AmbWave retains its existing shared effects.
Use dry exact recordings first, then compare the combined scene by listening.
No HALL import or effect algorithm change is hidden in this experiment.

Final software validation:

- `tools/build-ambwave-macos.sh` (calls `./build.sh`) passed. Application 443,952
  bytes; static RAM 90,368/98,304; pool 330,208/344,064. The application is 252
  bytes larger than the recorded Drone baseline; linker-reported RAM/pool totals
  are unchanged (the four control-clock bytes fit existing alignment).
- Package SHA-256: `2a7754ebe5c35e77996f75507e351ab9b63d86780d08d3f09b513c0f66fecae3`.
  Loader remains 6,693 bytes. No loader, package-format or recovery code changed.
- `tools/test-macos.sh` / `sh tests/run_tests.sh`: **ALL HOST TESTS PASSED**.
  92 golden renders unchanged; no health, voice/routing or CPU regressions.
  Storage, project, user-preset, editor, installer and generic browser checks pass.
  Optional DaisySP reference checkout is absent and that comparison was skipped.
- Target ISR weighted cost 40,454 versus 37,502 baseline (+7.9%, within 10%).
  New bounded helper: 91 instructions, 15 in loops, no divides/calls in its loop;
  independent weighted budget 15 passes. These are static guards, not hardware timing.
- 768,000 HOME/mixer layouts: zero lint findings or alignment failures. Both new
  sound screens were visually inspected. Generic UI alignment checks also pass.
- Additional AMBIENT_POC full-panel native/WASM tests pass for all ten TIDE and
  six PULSE choices, including exact native audio for the factory/new TIDE and
  PULSE audition sequences. Both PHYS sounds are reachable through real PRESET
  turns, latch their keys and produce audio.

## Listen

Local preview: `http://127.0.0.1:8796/`, serving
`build/sound-bath/preview`. Press Switch on. On track 1, advance PRESET eight
steps for BOWED METAL, then one more for BOWL STUDY. DENSITY is excitation:
BOWL starts at 0; raise it to sustain the struck resonator. SHAPE 50 / MOTION 0 /
SPACE 0 is the dry source reference. Track 3 is PULSE; select GLASS BELL with
three PRESET steps, hold notes and press PLAY for the existing sparse scheduler.

The comparison files and `tide-bowl-pulse-glass.wav` are under
`build/sound-bath/`. The example scene starts dry, adds the macros at the times
listed above, and releases at 70 seconds. Compare dry source first, then judge
whether AmbWave's shared wet mix preserves the desired musical experience.
User listening acceptance and hardware timing remain pending.

Restart the prepared preview with:

```sh
python3 -m http.server 8796 --bind 127.0.0.1 --directory build/sound-bath/preview
```


## Reproduce

Run the repository's configured build first, then its full host suite:

```sh
tools/build-ambwave-macos.sh
tools/test-macos.sh
EM_CACHE=/tmp/ambwave-bath-emcache sh tools/sound-bath/build.sh
python3 tools/sound-bath/verify.py --reference ../sound-bath/build/audition/audio
node tools/sound-bath/verify.mjs
cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src tools/sound-bath/scene.c -lm -o build/sound-bath/scene
build/sound-bath/scene build/sound-bath/tide-bowl-pulse-glass.wav
```

The Sound Bath path is an optional comparison input only. Firmware, native
renderer and scene have no runtime/build dependency on that neighboring tree.
Generated WAVs/binaries live under ignored `build/sound-bath`; regenerate rather
than checking large media into source. Keep native target timing, hardware
listening and musical acceptance separate from these software measurements.

## Hardware installation — 2026-10-20

After software validation, the user explicitly approved: “Yes, flash the Sound
Bath experimental build to my FM-1 now.” Installed the exact tested package
SHA-256 `2a7754ebe5c35e77996f75507e351ab9b63d86780d08d3f09b513c0f66fecae3`
using `tools/fm1_install.py build/felucca.fwsc --port Felucca --yes`. The installer
validated the package, completed writing to 100%, restarted the device, verified
its running identity `FM-1_900`, and exited successfully. Earlier no-installation
statements describe the pre-approval validation stage. Musical listening and
actual hardware timing acceptance remain pending.
