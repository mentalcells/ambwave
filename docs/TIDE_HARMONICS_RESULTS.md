# Tide harmonic-source experiment

2026-10-15, `codex/tide-dry-source`, compared against installed source `86d2167`.
The unchanged-upstream build and host test proof is recorded in AMBIENT_PLAN.md.

## Status: richness criterion not met

The user listened to the matched dry chord recordings and answered:
“Different, but not clearly richer.” This second candidate therefore also
fails the requested sound-quality criterion. Audible difference, stable
fundamental and measured harmonic evolution do not establish richer sound.
Keep it as an experiment for comparison; do not promote it as an accepted
improvement or proceed to installation on the strength of software tests.

The first source experiment was heard as “less harsh, more EQ, but not
necasarily richer.” Its saw/triangle blend is replaced, not layered with this
candidate. Earlier audio remains in `build/tide_dry_demo/`; the first source
snapshot and package are in `build/tide_triangle_reference/`. This candidate's
audio is in `build/tide_harmonics_demo/`.

The agent cannot listen to these recordings. This candidate has not met
the richness/expressiveness goal in user listening. Installation
on an FM-1 has not been authorized or performed.

## One synthesis improvement

Original TIDE now builds each of its two oscillator waveforms from the existing
interpolated sine primitive: a steady fundamental and four harmonic partials.
The second and third harmonics exchange strength; the fourth and fifth exchange
strength independently at a slower rate. Each pair retains its summed level,
and the fundamental coefficient remains fixed. All components are exact integer
harmonics of their oscillator's pitch, with no pitch drift added.

This aims to change internal harmonic relationships during a sustained note,
rather than move a broad saw/triangle spectral tilt. It is additive source
synthesis before the existing oscillator mix, drive, filter and amplitude
envelope, not another EQ or spatial stage. It introduces no additional voices,
unison, detuning, chorus, delay or reverb. More sine partials are calculated
inside the existing two oscillator slots; their cost is explicitly checked.

MOTION controls harmonic-exchange depth and the first exchange's speed. At zero
the source spectrum settles to stationary levels. At the default 25, the second
harmonic moves about +/-1800 around 6000 while the third makes the opposite
change around 5000; the fourth/fifth pair has one fifth of this movement. At
100 those ranges reach +/-3500 and +/-700. The two modulation periods are about
10 and 26 seconds at the default; note-dependent phases spread movement across
a chord. These are deterministic slow modulations, not random drift.

The source levels have about 6 ms of control smoothing. Each partial fades out
before its frequency reaches Nyquist, separately for the two oscillator pitches.
The fundamental stays at coefficient 16500; the two partial pairs sum to 11000
and 2400 before frequency taper, bounding the waveform's summed coefficients
below full scale.

SHAPE retains its filter mapping, DENSITY its existing oscillator mix/detuning,
and SPACE its delay/reverb curves. The existing MOTION filter drift remains;
the single-source renders disable it to isolate the new synthesis movement.
TIDE WARM, TIDE SILK, factory ANALOG presets, other sounds, voice allocation,
UI mappings and storage/package formats are unchanged.

## Dry listening procedure

SPACE=0 retains the previous experiment's chorus-send fix for all Tide variants.
It disables new chorus, delay and reverb sends even after DENSITY is edited.
Shared effect tails are allowed to finish on a running instrument; they are not
erased globally because other layers use those buses.

The recordings start with cleared buffers, keep other layers silent, and check
all three bus inputs and the wet return for exact zero throughout each take.
Original Tide is explicitly rendered with all three sends zero too. There are
no external effects. Each take holds for 20 seconds and releases for 10.

Reproduce after `tools/build-hasake-macos.sh`:

```sh
PYTHON=.venv/bin/python sh tools/tide-dry-compare.sh
```

The harness is compiled against both archived `86d2167` source and the working
tree. Recordings are in `build/tide_harmonics_demo/`:

- `default-chord`: C3/G3/E4, normal starting macros except SPACE=0.
- `single-source`: C3, DENSITY=0, MOTION=100, no oscillator noise or filter
  envelope/drift. This exposes the harmonic source without detuning or effects.
- `bright-register`: E5 at SHAPE=100, otherwise the starting macros, SPACE=0.
- `after/single-source-still.wav`: the isolated C3 with MOTION=0, for checking
  that the new source movement can be stopped.

`*-before-matched.wav` and `*-after-matched.wav` match the settled sustain
RMS over seconds 8–20 to 6000/32768 using one fixed gain per whole take. No EQ,
compression or time-varying normalization is added. `*-AB.wav` plays original
Tide for 30 seconds, one second of silence, then the new candidate for 30.

Judge the chord at 8–20 seconds for low-mid body and useful internal movement.
Then listen to the isolated oscillator and bright-register takes: does the
new harmonic movement sound rich and expressive, or simply different, hollow,
too simple or too obviously periodic? Listening decides acceptance. If useful
in recordings/emulator, a separately authorized box test should follow.

## Software evidence

- Firmware build: image 442532 bytes; static RAM 89836/98304; pool 330208/344064.
- Both render loops fit the original 585 target budget: factory 581, harmonic
  source 432; dispatch/control path 115. These are static instruction estimates,
  not device cycle measurements. No CPU threshold or golden hash is raised.
- In the isolated moving take, H2/H3 power ratio varies 0.21–40.67 during
  seconds 8–20 while measured fundamental amplitude changes by under 1%.
  At MOTION=0 the ratio stays 2.70–2.79, with fundamental spread about 0.2%.
  The focused test checks individual harmonic energies, not only brightness.
- Dry renders pass bus/return silence, signal bounds, DC and completed-release
  checks. The matched chord removes the raw RMS difference (7644 original,
  6896 candidate). All takes finish in silence.
- Separate `AMBIENT_POC=1` browser bundle `build/emu/tide-harmonics/` passes its
  integration checks and native/WebAssembly comparisons for layer transitions,
  pad chords and heavy-song audio. This does not change the listening verdict.
- Full host validation with the workspace Emscripten cache reports **ALL HOST
  TESTS PASSED**: 92 unchanged golden renders, no audio-health, routing or CPU
  failures, plus storage compatibility, UI and installer simulation checks.
  The four-layer macro-corner stress render takes 0.004 of host real time.
- Package SHA-256:
  `34b4df0f6814289929f81797672e6448231764de228b7562d4726952a9ada017`.

## Reference for further work

After judging this candidate “Different, but not clearly richer,” the user
provided https://www.youtube.com/shorts/7qTpiEUz5EQ as the sound reference.
That link alone does not establish its synthesis architecture or dry/effected
routing. Do not claim to have heard or matched it without accessible audio
and a listening assessment.
The source audit and isolated Rhodes/Voices pad auditions are recorded in
`AMB1_DRY_REFERENCE.md`; no third Tide implementation is selected from the link alone.

No connected device, presets, samples, projects or backups were written.
