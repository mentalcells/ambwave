# Factory sound audition — Phase 1

2026-10-20. Branch `codex/factory-sound-audition`, continuing the role/patch
separation on the `ambient-prototype` base. Existing Tide experiments remain
intact and opt-in; no experiment has become a factory default.

## Try it

Browser: http://127.0.0.1:8794/ (`build/emu/audition/`). Click Switch on,
select a track with ALGORITHM and use PRESETS for the sounds below. HOME +
PRESETS changes role. In the browser, Shift-click HOME to hold/release it.
Changing sound deliberately clears the old held notes; retap the notes.

| Role | Stable patch ID | Display name | Existing source |
| --- | ---: | --- | --- |
| Tide | 0 | BODY PAD | Original custom FM6 Tide, unchanged |
| Tide | 1 | WARM PAD | Original curated ANALOG variation, unchanged |
| Tide | 2 | SILK PAD | Original curated FM6 variation, unchanged |
| Tide | 3 | FM SOFT PAD | FM6 factory PAD, index 4; SOFT PAD VMEM |
| Tide | 4 | AN SOFT PAD | ANALOG factory SOFT PAD, index 1 |
| Tide | 5 | AN STRINGS | ANALOG factory STRINGS, index 11 |
| Tide | 6 | AN PWM STR | ANALOG factory PWM STR, index 3 |
| Tide | 7 | CHOIR AAH | FORMANT factory CHOIR AAH, index 0 |
| Pulse | 0–2 | WARM PLUCK / SOFT MALLET / BELL PLUCK | Original PHASE sounds, unchanged |
| Pulse | 3 | GLASS BELL | FM6 factory BELL, index 1 |
| Pulse | 4 | FM MARIMBA | FM6 factory MARIMBA, index 5 |
| Pulse | 5 | FM TINE EP | FM6 factory TINE EP, index 0 |

Both STRINGS and PWM STR are included as separate audition candidates. The
original IDs, boot arrangement and default mixer levels are unchanged. FM and
AN prefixes distinguish factory sounds from the original curated pad variants.

## Scope and sound ownership

`ambient.c` catalog entries now identify a factory preset or the existing
curated initializer. Candidates load through `ui.c:apply_preset_to()`, retaining
factory engine parameters, track ADSR, filter envelope, voice mode, modulation,
distortion and FX sends. FM6 keeps the actual factory VMEM, including operator
envelopes and native LFO settings. No patches or synthesis code were retuned.

Role configuration still handles HOLD/SCATTER/SEQUENCE, held-note limits,
speed/probability and transport. Candidates bypass the original Tide envelope
rewrites and PHASE wave/DCW/detune/random-modulation overrides. Existing
sound predicates are bounded to the original patch IDs; an imported Formant
patch cannot accidentally enter the Warm/Silk paths.

For this audition the new candidates' SHAPE and MOTION, and Tide DENSITY, are
intentionally unmapped and display FACT. Turning them leaves the factory tone
unchanged and displays FACTORY TONE. This is an explicit Phase 1 limitation,
not a new macro design. Original sounds retain all four existing macros.

New Pulse DENSITY changes only event probability. New SPACE is centered at 50:
the original factory chorus/delay/reverb sends and unity dry gain. Below 50 it
scales sends toward zero; above 50 it raises sends and reduces dry gain. Returning
to 50 restores the factory sends exactly. Distortion stays as supplied by the
factory preset. Effects algorithms, shared-bus settings and `fx.c` are unchanged.

This means patches may have different loudness, brightness, native vibrato and
decay. They have not been normalized, softened or made identical. In HOLD,
a factory pluck can decay even while its input note remains held. These are
expected source differences for listening, not failed note ownership.

## Compatibility and resources

- Four tracks and existing selection/mix/mute/clear gestures remain available.
- Patch changes retain mode, explicit speed, pattern, level, pan, ambient mute
  and global FX; old live notes are cleared, as before.
- Engine implementations, effects, official DX7 banks, loader, bootloader,
  recovery and package format were not changed.
- Existing projects serialize the actual engine parameters and FM6 data.
  Every new candidate passes an existing-format serialized round trip. Loads
  leave curated HOME ownership, as before: this still does not implement scene
  recall or restore the curated role/patch identity.
- All layers and release tails share eight voices. Dense sustained combinations
  beyond that budget can steal voices. Factory FM tails may overlap longer
  than original Pulse tails; no envelope shortening was applied to hide that.
- No device was flashed or user presets/projects/samples overwritten.

Firmware build: `tools/build-ambwave-macos.sh` (calls `./build.sh`), image
442384 bytes, +756 bytes over the role-separation build. Static RAM remains
89948/98304; pool remains 330208/344064. No new per-track or per-voice allocation.
Package: `build/felucca.fwsc`; SHA-256
`2e9cfe839ae843605631ca49e45e8072da51f29132eeff7184d8017af5d1a07c`.
Building this package does not authorize installing it.

## Validation

- `tools/test-macos.sh` / `sh tests/run_tests.sh`: ALL HOST TESTS PASSED.
  All 92 golden renders unchanged; zero health, voice/routing or CPU-budget
  failures. Target-loop budgets, storage, UI and default emulator checks pass.
  The optional DaisySP reference checkout is absent and its check is skipped.
  The expanded factory audio/project checks were also compiled and run separately.

- `factory_audition_test.c`: all eight candidates on every track in all three
  modes match a plain factory load's non-track sound parameters, FM6 VMEM and
  FX sends. Unmapped controls do not change them; Pulse Density edits timing
  only; SPACE 0 is dry and 50 restores factory sends.
- All eight dry candidate renders equal their plain factory reference bit for
  bit, with equal gain and deterministic noise seeds. Mixer slew is initialized
  equally for this source comparison.
- Candidate selection, HOLD/SCATTER/SEQUENCE audio, STOP/clear release, undo/redo
  and existing-format project save/restore pass.
- Original eight sounds: 384 patch/mode/macro-corner renders are bit-identical
  to a snapshot of the working source before this change.
- Four duplicate candidate tracks with eight notes, high sends, and every
  candidate substituted into the original four-layer performance pass bounded
  output, voice-budget and sustained-gate checks. Worst duplicate host cost
  observed about 0.005 of real time; this is not an on-device CPU measurement.
- HOME/MIX layout sweep: 491520 screens, zero lint and alignment findings.
  Choir and Glass Bell screens with FACT labels were also visually inspected.
- AMBIENT_POC=1 browser tests cover all candidate selections and audible held
  input. Native/WebAssembly checks cover the existing layered performance,
  original Silk/Warm chords and factory audition sequences.

Hardware listening and actual target timing with these new combinations remain
unverified. This build is for choosing sounds, not a claim of final sound tuning.
