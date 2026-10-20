# Tide P0a: opt-in independent breathing experiment

2026-10-18, branch `codex/tide-p0a-breathing`, based on `ambient-prototype`
at `828ceaa`. **Hardware listening: movement improved, richness goal not met.
Factory Tide is unchanged.**

## Scope and result

One conservative implementation pass adds independently phased amplitude and
FM-index movement to existing held Tide notes. It adds no operators, voices,
detune, filter, stereo placement or effects. Its purpose is to test movement;
it is not presented as proof of a richer static source. No further breathing
tuning is justified until the first listening comparison is convincing.

`TIDE_BREATH_EXPERIMENT` defaults to **0**. Code and state are compiled out
of normal firmware. The normal target app was rebuilt and compared byte for
byte with the pristine `828ceaa` app: identical. The candidate is isolated in
`build/p0a/candidate-target/`; normal build outputs were restored with the flag
off. No hardware installation occurred during preparation; the subsequently
authorized installation is recorded below. Nothing was published or written
to user preset, sample or project storage.

Only the original FM Tide in HOLD is eligible in the opt-in build. Warm, Silk,
other sounds, SCATTER and SEQUENCE retain their existing behavior. Original
Shape, Density, Space, shared DRIFT and all parameter mappings are untouched.
Motion also scales the extra breath in the experiment; Motion0 bypasses it
exactly after its existing block ramp. The four-note panel latch and global
eight-voice limit are unchanged. No stored format changes.

## Listening comparisons

Open the [local listening page](http://127.0.0.1:8791/) while the preview server
is running, or use the generated files below. **A is always the unchanged
baseline; B is the opt-in candidate.** Each short take holds for 40 seconds,
then releases for 12 seconds. Combined A/B files have a one-second silent gap.

| Comparison | Listen | Evaluate |
| --- | --- | --- |
| Single sustained D3, MIDI 50 | [A then B](../build/p0a/listening/single-AB.wav) | Body/warmth separately from the new breath; does it sound like useful expression or obvious level modulation? |
| Four-note Dm7, MIDI 50/57/65/72 | [A then B](../build/p0a/listening/chord-AB.wav) | Fullness and blend versus internal movement. |
| Higher chord, MIDI 62/69/77/84 | [A then B](../build/p0a/listening/high-AB.wav) | Does the candidate become thin or too bright at a higher register? |
| Single note, five-minute hold | [A](../build/p0a/listening/single-long-A-matched.wav), [B](../build/p0a/listening/single-long-B-matched.wav) | Long-term movement; each file includes 12 seconds of release. |
| Chord, five-minute hold | [A](../build/p0a/listening/chord-long-A-matched.wav), [B](../build/p0a/listening/chord-long-B-matched.wav) | Does independent note balance remain musical over time? |
| Motion0, single / chord | [Single A/B](../build/p0a/listening/single-still-AB.wav), [chord A/B](../build/p0a/listening/chord-still-AB.wav) | Static source control: the paired files are sample-identical. |
| Motion100 chord | [A then B](../build/p0a/listening/chord-motion100-AB.wav) | Existing high-Motion behavior plus the maximum new breath; 60-second hold. |
| Held-note macro changes | [A then B](../build/p0a/listening/chord-gestures-AB.wav) | Shape/Density edits at 10 s, Motion100/Shape at 20 s, Motion0/Density at 30 s. Identical gestures in A and B. |

Start with the first two comparisons. Judge two independent questions:

1. Does B have a convincingly fuller/warmer body at matched loudness?
2. Is B's evolving movement more musical and interesting, without pumping?

A positive answer only to the second question does **not** satisfy the original
richness goal. If the first impression is unconvincing, retain the baseline
and move to a separate static-voicing experiment rather than repeatedly
intensifying breathing. No listening verdict has been inferred from tests.

### Fairness and measurements

Every take runs in a fresh process with identical note events, velocity 100,
register, mixer level and deterministic seeds. All spatial sends/returns are
verified zero, and dry L/R are identical. The recorded raw gain is unchanged.
Matched files use **one constant gain per entire take**, targeting RMS
6000/32768 over seconds 8 through note release. No breath-by-breath gain
normalization, EQ, compression or added spatial processing. All raw takes are
retained beside the matched files.

| Default Motion25, 40-second hold | Baseline A | Candidate B |
| --- | ---: | ---: |
| Single-note raw sustain RMS | 1449.1 | 1304.8 |
| Single-note matching gain | +12.34 dB | +13.25 dB |
| Single-note one-second RMS range | 0.29 dB | 2.08 dB |
| Chord raw sustain RMS | 2878.1 | 2594.3 |
| Chord matching gain | +6.38 dB | +7.28 dB |
| Chord one-second RMS range | 0.33 dB | 0.55 dB |
| Chord energy in 80–300 Hz | 45.27% | 44.97% |
| Chord energy in 300–1500 Hz | 54.67% | 54.97% |

These show more dynamic movement, especially in the isolated note, and little
change in average spectral distribution. They do not establish added static
body. The short matched chord peaks at 19,417/19,601 (A/B), below clipping.
At unchanged mixer gain, the solo note/chord takes have no measured limiter
reduction, DC below one sample unit, zero final tail peak and zero remaining
voices. The five-minute chord's one-second level range changes from 1.59 to
2.01 dB; this is intentionally restrained rather than synchronized pumping.

Windowed spectral proportions and level ranges are diagnostic descriptions,
not sound-quality scores. The higher matching gain compensates for the
candidate's attenuating breath; it is not baked into firmware or presets.
Exact values, raw hashes, headroom checks, invariant proofs and CPU trials
are in [measurements.json](../build/p0a/listening/measurements.json).

## Implementation

[eng_fm6.c](../firmware/src/eng_fm6.c) contains the compile-time switch and
the small per-voice state array; [ambient.c](../firmware/src/ambient.c)
limits eligibility to original Tide HOLD. Each note starts with a deterministic
phase derived from note, slot and voice index, independent of the shared RNG.
Its nominal cycle at Motion25 is 12–25 seconds; each wrap chooses a new period.
Motion scales rate through the existing block-time update, keeping long cycles.

A sine-derived breath attenuates by at most about 2 dB at Motion25 and 3 dB
at Motion100. It also supplies roughly ±0.6 dB of FM modulator-level movement
at Motion25. It never changes carrier frequency. Existing FM index/operator
gain and output amplitude ramps smooth the changes. Motion0 and excluded
sounds use exact-unity arithmetic, avoiding a fixed-point gain loss.

The 16-byte state per allocated FM voice is reset at each note-on/reuse.
Released notes finish through existing FM envelopes; panic/mode/sound changes
retain existing note-release behavior, and a new note resets its breath.
There is no second effect network, heap allocation or extra sample loop.

## Automated validation

The full host suite with `CC='cc -DTIDE_BREATH_EXPERIMENT=1'` completed with
**ALL HOST TESTS PASSED**. It used a fresh opt-in target build:

- 92 unchanged generic golden renders; zero health, voice/routing and CPU
  budget failures. No golden hashes or budget thresholds were changed.
- New P0a checks: bounded gain, invariant operator pitch, unchanged patch and
  macro parameters while held, independent phases, Motion0 unity, four-note
  latch cap, per-slot eligibility, excluded sounds/modes and clean release.
- Original ambient four-layer macro corners, MIDI ownership/voice pressure,
  storage/presets/projects, UI/input and installer simulations pass.
- 184,320 ambient HOME/MIX screens, zero lint/alignment failures.
- Default-build audio exactly matches pristine source for single/chord/high,
  layered mix, eight Tide voices, Warm, Silk and SCATTER. Motion0 single/chord
  are identical with the feature compiled in; Warm/Silk/SCATTER also match.
- Candidate repeated rendering is deterministic. Eight sustained FM Tide
  voices across all four slots remain bounded and release completely. That
  artificial stress mix activates the baseline limiter by 0.72 dB versus
  0.16 dB in B at the same fader settings; no extra voices are introduced.
- Explicit opt-in AMBIENT_POC browser UI/integration tests pass. A dedicated
  native/WebAssembly check matches **1,764,000 frames** of dry held-chord
  audio bit for bit, with Motion25/100/0 edited through real panel controls.
- Optional DaisySP reference test and official V15 restore simulation were
  skipped because their external checkout/package is absent.

Logs are retained in `build/p0a/validation/`. The test/render tools do not
connect to hardware or overwrite presets, samples, projects or backups.

## Resources

Fresh target builds used the same local JieLi/SDK toolchain and flags, with
AMBIENT_POC enabled. A separate pristine source snapshot provided A.

| Target resource | Pristine / normal build | Opt-in candidate | Delta |
| --- | ---: | ---: | ---: |
| App image | 441,300 B | 441,824 B | +524 B |
| Static RAM, data+BSS | 89,932 B | 90,320 B | +388 B |
| Buffer pool | 330,208 B | 330,208 B | 0 |
| Static RAM free | 8,372 B | 7,984 B | -388 B |
| Pool spare after mandatory 8 KiB reserve | 5,664 B | 5,664 B | 0 |

The state array itself is 384 B; linker layout accounts for the measured
388-byte total RAM delta. Normal builds have no extra state or code. The
normal app SHA-256 is
`c6099c65ddff84624b07f787fe32c33cf489b3c252a87db44493ea9a9106877d`;
candidate app SHA-256 is
`c36f492ce7fffbf6d7383ef7ed0b434588c56eeae3b6ec9b23c570de1f2f028d`.

Median kernel-counted host DSP instructions/frame, three 20-second trials
after warmup, excluding file output:

| Scenario | A | B | Increase |
| --- | ---: | ---: | ---: |
| One held note | 469.62 | 472.16 | 0.54% |
| Four-note Tide chord | 788.02 | 798.15 | 1.28% |
| Four-layer mix | 1082.12 | 1092.10 | 0.92% |
| Eight Tide voices across four slots | 1440.54 | 1485.61 | 3.13% |

All target loop-cost checks pass unchanged thresholds. `fm6_render` cost is
378 versus the recorded 372 baseline; operator sample-loop costs remain
72/26. Audio ISR heuristic cost remains 38,515 against budget 37,502.
The noinline breath helper adds control-rate work outside those sample loops.
Neither these heuristic counts nor host percentages are a measurement of
FM-1 CPU utilization. **Hardware worst-case half-buffer timing, late counters
and overload shedding remain unmeasured**. The installation recorded below
does not establish those measurements or listening acceptance. Do not promote
it to the default on software evidence alone.

## Reproduce, disable and next decisions

With the usual build dependencies:

```sh
# Normal build: experiment disabled (the default).
TIDE_BREATH_EXPERIMENT=0 tools/build-ambwave-macos.sh

# Opt-in target measurement only; does not install firmware.
TIDE_BREATH_EXPERIMENT=1 tools/build-ambwave-macos.sh
EM_CACHE="$PWD/.toolchains/emscripten-cache" \
  CC='cc -DTIDE_BREATH_EXPERIMENT=1' tools/test-macos.sh

# Listening pack; PYTHON must have NumPy. Use a new output directory.
PYTHON=python3 sh tools/tide-breath-compare.sh build/p0a/new-listening

# Candidate browser; normal browser omits EMU_CFLAGS.
AMBIENT_POC=1 EMU_CFLAGS=-DTIDE_BREATH_EXPERIMENT=1 sh web/emu/build.sh
```

The comparison script archives the public baseline revision and compiles the
same renderer against A, normal current source and opt-in source. It refuses
to replace an existing listening pack. For the dedicated browser equivalence
check, compile `tools/tide-breath-web-check.c` with both flags, render its native
`.f32`, then pass candidate wasm and that file to
`node tools/tide-breath-web-check.mjs WASM NATIVE.f32`.

Disable/reverse by keeping the flag unset or 0; no data migration or preset
rollback is needed. Factory preset selection and engine numbering are unchanged.

Only P0a is implemented. The revised next-work priorities are recorded in
[the assessment](DESIGN_GAP_ASSESSMENT.md): assess cheap stereo decorrelation
earlier, retain coherent four-layer curation/level balancing, and treat reliable
performance save/restore separately and ahead of long scene morphs. None of
those features was implemented as part of this experiment.

## Authorized hardware installation — 2026-10-18

The user explicitly requested flashing P0a, then asked to retry after connecting
the device. The FM-1 identified as `FM-1_900 [running]` on MIDI port `Felucca`.
The candidate package checksum was reverified immediately before installation:
`f405ff4a031cf2f2aa75ee431b45f3358c804b8b159d222ad14a4fde89e49833`.

Installed with `tools/fm1_install.py build/p0a/candidate-target/felucca.fwsc
--port Felucca --yes`. The existing installer checked the package, wrote to
100%, restarted the device and reported `done: the FM-1 runs FM-1_900`, exiting
successfully. The log is `build/p0a/validation/hardware-install.log`.
No user-data erase was requested. Installation success and the shared device
identity do not establish sound approval or real-time timing headroom.
Factory source defaults remain disabled; this device now runs the opt-in
candidate for listening evaluation.

## Hardware listening outcome

The user reported that movement is more noticeable, but the sound still is
not rich enough. This distinguishes improved animation from static fullness;
P0a is not accepted as the richness solution and remains an opt-in experiment.
Stop tuning breathing for now. The next isolated dry-source audition is
[P0b static voicing](TIDE_P0B_RESULTS.md), with Motion0 and Space0, without
altering the installed firmware or factory baseline.
