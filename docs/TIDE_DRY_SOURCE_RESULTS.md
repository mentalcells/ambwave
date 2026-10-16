# Tide dry source experiment

Historical first candidate. The replacement harmonic-source candidate is
recorded in `TIDE_HARMONICS_RESULTS.md`. Its recordings use a separate directory.

2026-10-15, `codex/tide-dry-source`, based on installed source `86d2167`.
Milestone 0's unchanged-upstream build/test evidence remains in AMBIENT_PLAN.md.

## Status: richness criterion not met

User listening feedback on the dry chord comparison: “less harsh, more EQ,
but not necasarily richer.” The reduced harshness is useful, but this does not
accept the candidate as a richer or more expressive instrument. Do not promote
this experiment on the strength of its measurements or passing tests.

The next hypothesis should target changing relationships among low and middle
harmonics during sustain, rather than another broad change in spectral slope.
Evaluate that as a replacement source experiment against the original dry
TIDE reference, not as more processing stacked onto this candidate. It has
not been implemented or validated by this first experiment; it is the subject
of the separate replacement experiment.

The saw/triangle source snapshot and package are preserved locally under
`build/tide_triangle_reference/`; its recordings remain in `build/tide_dry_demo/`.
The agent
cannot listen to the generated audio in this environment. Measurements establish
source activity, routing and signal health; they do not establish warmth,
richness or musical expressiveness. No hardware installation was requested or
performed. Keep the installed revision as the listening/rollback reference.

## One synthesis change

Original TIDE's two ANALOG saw oscillators now each blend with Felucca's existing
triangle waveform at the **same phase and pitch**, before their existing mix,
drive and filter. The triangle's fundamental is phase-aligned with the saw's.
The mean blend is 55% triangle: retain some saw even harmonics while increasing
the relative weight of the fundamental and reducing the upper harmonic slope.
This is waveform shaping of the existing oscillators, with no additional
oscillator voices, unison, detuning or spatial processing.

MOTION adds two slow sine movements to this harmonic blend. The first period
ranges roughly 18 to 4.5 seconds; the second is about 26 seconds. Their unequal
periods and note-dependent phases keep chord tones from moving together. This
is deterministic slow movement, not random drift. At MOTION 25 the blend varies
by up to 15 percentage points, at 100 by up to 30; at zero it is stationary.
The existing filter DRIFT remains. The single-source audition disables it to
prove that the new sustained movement comes from the oscillator waveform.

SHAPE retains its filter mapping, DENSITY retains its existing oscillator mix
and detuning, and SPACE retains its delay/reverb curves. TIDE WARM and TIDE SILK
have no synthesis change. The experiment is deliberately confined to original
TIDE, so their existing tones remain available as alternatives.

## Truly dry comparison

Previously SPACE=0 still left DENSITY's chorus send active. All three TIDE
variants now set chorus, delay and reverb sends to zero at SPACE=0. Turning
DENSITY while dry cannot restore chorus. Raising SPACE restores its existing
DENSITY-derived chorus amount without changing oscillator settings.

FX buffers remain shared. Turning SPACE down stops new sends; an existing tail
or another layer's effects can still be heard. The comparison starts from fresh
buffers, plays only TIDE, disables performance FX, and checks the actual three
bus inputs **and wet return** for zero throughout every sample of sustain and
release. Reference renders explicitly zero all three sends too, so the old
chorus behavior cannot bias the comparison. No external effects are used.

## Listening material

The first audition used this command with the first candidate's sources:

```sh
PYTHON=.venv/bin/python sh tools/tide-dry-compare.sh
```

This compiles the same harness against `git archive 86d2167` and the current
sources. At the time it wrote `build/tide_dry_demo/`; the current script writes
the replacement candidate to `build/tide_harmonics_demo/`. Re-running the current
script does not recreate this first candidate. It never accesses hardware or user
storage. Before and after use the same pitches, velocity, level and macro
positions. Three cases each hold for 20 seconds, then release for 10 seconds:

- `default-chord`: C3/G3/E4, normal starting macros except SPACE=0.
- `single-source`: C3, DENSITY=0 (one oscillator, no detune), MOTION=100,
  oscillator noise and filter envelope/drift disabled.
- `bright-register`: E5, SHAPE=100, otherwise the starting macros, SPACE=0.

Raw files are in `before/` and `after/`. `*-before-matched.wav` and
`*-after-matched.wav` use a single fixed gain per take, matching RMS over seconds
8–20 to 6000/32768. This leaves onset, internal dynamics and release intact;
there is no compression, EQ or time-varying normalization. `*-AB.wav` plays the
30-second reference, one second of silence, then the 30-second experiment.
`measurements.json` records gains, late spectral-change proxy and final silence.

Listen especially at 8–20 seconds, after the attacks settle. Judge low-mid body,
upper-harmonic harshness, whether the evolving tone is interesting or merely
duller, and whether a chord stays expressive with no spatial support. Check
the single-source and upper-register comparisons before accepting the change.
Then assess live SHAPE/MOTION/DENSITY response and layered CPU behavior on the
FM-1 only after explicit installation approval in that conversation.

## Software evidence

- Hasake build: image 441896 bytes; static RAM 89836/98304; pool 330208/344064.
- Package SHA-256:
  `6e06024b023ab0460e9f0e3c7271ca1c384bd576ee06c1a004be06c8cc7c3511`.
- New dry tests check both macro-edit orders, all three TIDE variants, zero
  effect-bus input/return, bounded output, DC, completed release and continuing
  harmonic change during seconds 8–20 with filter drift/detuning disabled.
- Raw default-chord sustain RMS is 7644 before / 7953 after; matching removes
  this roughly 0.34 dB difference. Single-source brightness proxy is essentially
  stationary before (0.00064) and varies 0.00038–0.00049 after. This establishes
  continued source evolution, not a listening verdict.
- ANALOG uses one shared implementation specialized into factory and TIDE
  loops. The target check explicitly checks both against the original 585
  budget: factory 581, TIDE 238; dispatch/control path 265. These are static
  instruction estimates, not device cycle measurements. Existing CPU limits
  and audio golden hashes are not raised.
- `EM_CACHE="$PWD/.toolchains/emscripten-cache" tools/test-macos.sh` reports
  **ALL HOST TESTS PASSED** for the final source: 92 unchanged golden renders,
  zero health/routing failures, zero CPU entries over budget. Storage migration,
  installer simulation, UI checks (including 184320 HOME/MIX layouts), four-layer
  macro corners and native/WebAssembly regression comparison pass. Four-layer
  worst host render time is 0.004 of real time; hardware margin remains untested.
- Separate `AMBIENT_POC=1` browser bundle: `build/emu/tide-dry/`. Its integration
  checks pass, including native/WebAssembly bit-for-bit comparison of the heavy
  song, four-track layer transitions, and SILK/WARM pad onset/sustain/release.
  To audition, serve `build/emu` locally, open `/tide-dry/`, lower SPACE to zero
  **before playing**, and keep the other layers silent. The default controls
  and startup settings are preserved. There is no server started by this work.

No bootloader, loader, recovery, package format, presets, samples, projects or
device backups were modified. A built package does not authorize installation.
