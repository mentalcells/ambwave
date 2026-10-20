# Tide P0b: static FM voicing auditions

2026-10-18. **Offline listening experiment only; no firmware changes or
installation. No candidate has been accepted or made default.**

P0a hardware feedback separated the issues: movement is more noticeable,
but Tide still lacks richness. These auditions change static FM voicing with
Motion0 and Space0. P0a is compiled out. All candidates use existing FM6 DSP,
the same note events, velocity, envelopes and voice limits.

## Listen

[Open the P0b comparison page](http://127.0.0.1:8791/p0b/) while the local
preview server is running. Start with A/B. C is a secondary diagnostic if B
still lacks body, rather than a presumed improvement.

| Sound | Primary: A → B | Optional: B → C |
| --- | --- | --- |
| Single D3, MIDI 50 | [Single A/B](../build/p0b/listening/single-AB.wav) | [Single B/C](../build/p0b/listening/single-BC.wav) |
| Dm7, MIDI 50/57/65/72 | [Chord A/B](../build/p0b/listening/chord-AB.wav) | [Chord B/C](../build/p0b/listening/chord-BC.wav) |
| Higher chord, MIDI 62/69/77/84 | [Higher A/B](../build/p0b/listening/high-AB.wav) | [Higher B/C](../build/p0b/listening/high-BC.wav) |
| 90-second held chord | [Long A/B](../build/p0b/listening/chord-long-AB.wav) | [Long B/C](../build/p0b/listening/chord-long-BC.wav) |

The first three hold for 30 seconds and release for 12. A one-second gap
separates takes in a combined file. Individual raw and matched A/B/C files
are retained in `build/p0b/listening/`.

Judge **body/fullness, warmth, and definition**. More brightness, a louder
attack or beating alone is not success. In particular, check whether B adds
useful harmonic body or merely makes Tide sound more metallic/organ-like.
Check higher notes for harshness and the chord for muddiness.

## Recipes and isolation

| Variant | Recipe | Active operators |
| --- | --- | ---: |
| **A: current Tide** | Actual baseline patch: one 1:1 FM pair and a quiet unmodulated second harmonic. | 3 |
| **B: revised voicing** | Same carrier topology. Modulator ratio changes from 1:1 to 2:1; its output level 66 → 72. Existing second harmonic level 75 → 78. Fundamental carrier remains 99. | 3 |
| **C: secondary carrier test** | B plus the otherwise silent third pair's fundamental carrier, level 82, detune 7 → 8 (one native step). Its modulator remains silent. | 4 |

B changes the harmonic structure rather than repeating the previously
unsuccessful second-harmonic-gain-only test. C isolates the addition of one
quiet carrier relative to B. It may create slow beating despite Motion0;
that distinction is explicit, and C is not a detuning-free static control.
Attack/release and existing macros are held constant rather than changing
several sound-design dimensions at once.

No reverb, delay, chorus, additional filter, stereo spreading, new synthesis
engine, runtime breath or modulation process is added. L/R equality and zero
spatial sends/returns are checked. No recording in the listening comparisons
activates the master limiter. One fixed gain per entire take matches RMS over
seconds 8 through release to 6000/32768. No time-varying normalization is used.

At unchanged faders B's 30-second chord is roughly 2.35 dB louder than A;
matching removes that advantage. Matching gains are +6.58 dB (A), +4.23 dB
(B), +4.12 dB (C). Matched chord peaks are 19,892 / 15,816 / 17,492, below
clipping. Candidate B has more upper harmonic energy in the diagnostic
spectrum; that establishes a different source, not a subjective richness win.
Full raw/matched metrics are in
[measurements.json](../build/p0b/listening/measurements.json).

## Automated checks and resources

- **108 patch/pitch/macro corners passed**: A/B/C, notes 36/50/72/96,
  Shape0/50/100 and Density0/50/100. Checks cover dry mono routing, bounded
  output/DC, release silence, no remaining voices, parameter integrity and
  the intended three/four active operators.
- A's chord is byte-identical to a separate renderer compiled against pristine
  `828ceaa`. B repeats deterministically. Factory patch code has not been edited.
- A clean invocation of the standalone comparison script reproduced all 12
  raw single/chord/high/long recordings byte for byte.
- A/B/C each sustain eight voices across all four slots and release cleanly.
  The global cap remains eight; C's additional operator does not consume
  another allocated voice.
- The eight-voice stress mix activates the master limiter by about 0.88 /
  1.80 / 2.48 dB for A/B/C at unchanged faders. This is a headroom cost to
  resolve before integration, not evidence of desirable warmth. The actual
  single/chord/higher/long listening takes have zero limiter reduction.

Median kernel-counted host DSP instructions/frame, three 20-second trials
after warmup, with file writing excluded:

| Scenario | A | B | C | C versus A |
| --- | ---: | ---: | ---: | ---: |
| One note | 469.73 | 469.68 | 488.52 | +4.00% |
| Four-note chord | 788.08 | 788.05 | 863.63 | +9.59% |
| Eight Tide voices / four slots | 1318.78 | 1327.81 | 1480.62 | +12.27% |

B's cost is essentially unchanged for one/four notes and +0.69% in this
eight-voice stress case. C pays for another active operator even though its
state already exists in FM6's allocated six-operator notes. There are no
new runtime arrays or buffers in these recipes. C needs a meaningful audible
advantage to justify its higher CPU cost; it is not recommended for hardware
integration merely because it passes host health checks.

These are host cost comparisons, not measured FM-1 utilization. No P0b target
package has been built: the recipes currently exist only in an offline
renderer. The previously verified factory app and the device's installed P0a
remain unchanged. A selected patch must receive a separate target build,
headroom/CPU-budget check and explicit hardware-test approval. The earlier
full P0a host-suite result is not represented as a new integrated P0b run.

## Reproduce and next decision

With existing generated firmware headers and a Python with NumPy:

```sh
PYTHON=python3 sh tools/tide-voicing-compare.sh build/p0b/new-listening
```

The script compiles the shared dry renderer and corner test, archives pristine
`828ceaa` for an independent reference and generates the complete comparison
pack. It refuses to overwrite an existing pack. Log for this run:
`build/p0b/validation/audition-tests.log`.

[tide-voicing-patches.h](../tools/tide-voicing-patches.h) holds the isolated
recipes; [tide_voicing_test.c](../tests/tide_voicing_test.c) tests their
corners. Firmware never includes the recipe header. The shared P0a renderer
has an optional compile-time setup hook; its default operation is unchanged.

The next decision is listening to A/B first. If B has convincingly more body
at matched level, refine that direction before adding carriers. If not, C
provides a single additional-carrier comparison. If neither works, record
the result and revisit voicing rather than combining these changes with
breathing or effects to disguise the failure. No listening acceptance is
claimed from metrics, and no hardware installation is part of this task.

## Listening follow-up — 2026-10-19

The user heard B as more harmonic/bright, with A warmer and more grounded;
B's single note felt more flute-like. This does not meet the richness goal.
C inherits B's revised voicing. The new [D audition](TIDE_D_RESULTS.md)
preserves A's original patch and adds only one quiet detuned fundamental.
