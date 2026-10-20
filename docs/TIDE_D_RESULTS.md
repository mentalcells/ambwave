# Tide D: original warm source plus a quiet carrier

2026-10-19. **Offline audition only. Factory Tide and installed firmware are
unchanged. Listening acceptance is pending.**

The user found A warmer and more grounded; B sounded brighter, more harmonic
in chords and more flute-like as a single note. D starts from A, not B or C.

## Listen

[A/D listening page](http://127.0.0.1:8791/d/). The old P0b page also links here.
A always plays first, D second, with a one-second silent gap.

| Comparison | Audio |
| --- | --- |
| Single D3, MIDI 50 | [A then D](../build/p0d/listening/single-AD.wav) |
| Dm7, MIDI 50/57/65/72 | [A then D](../build/p0d/listening/chord-AD.wav) |
| Higher chord, MIDI 62/69/77/84 | [A then D](../build/p0d/listening/high-AD.wav) |
| 90-second held chord | [A then D](../build/p0d/listening/chord-long-AD.wav) |

Short takes hold 30 seconds and release for 12; long takes hold 90 and release
for 12. Judge whether D preserves A's grounding and adds body. Separately
judge whether the extra beating is useful, too chorused or distracting.

## Exact change and controls

D changes only two bytes of A's patch: the previously silent third-pair
carrier output level 0 → 82, and its detune 7 (neutral) → 8 (one native,
pitch-dependent step). Its ratio stays 1:1 and its modulator stays silent.
The original FM pair, second harmonic, envelopes, feedback and every other
patch byte are unchanged; the corner test explicitly verifies this restriction.
D has four active operators versus A's three, with no additional voice.

Motion0/Space0, P0a compiled out, identical events/velocity/levels. No LFO,
breathing, EQ or spatial processing. Fixed detune can cause beating despite
Motion0. Both channels are identical and all spatial sends/returns zero.
One fixed gain per take matches RMS over seconds 8 through release to
6000/32768. Raw and individual matched takes are retained beside the A/D files.

The short chord's raw RMS is 2812.4/2881.1 (A/D), about +0.21 dB before
matching. Matching gains are +6.58/+6.37 dB; matched peaks 19,892/22,823.
All listening takes have zero limiter reduction, no clipping, zero final tail
peak and no remaining voices. These establish signal health, not richness.

## Validation and resources

- **144 patch/pitch/macro corners pass**: A/B/C/D; notes 36/50/72/96;
  Shape0/50/100 and Density0/50/100. Dry routing, output/DC bounds, release,
  parameter integrity, operator count and voice cap are checked.
- D preserves every original patch byte except the two allowed above and
  renders deterministically. A matches the pristine `828ceaa` renderer.
- Eight held Tide voices across four slots remain bounded and release cleanly.
  That stress mix activates the limiter by about 0.88 dB in A and 1.58 dB in D
  at unchanged faders: a headroom cost to address before integration. Actual
  listening comparisons have no limiting.

Median kernel-counted host instructions/frame, three 20-second trials after
warmup, file output excluded:

| Scenario | A | D | Increase |
| --- | ---: | ---: | ---: |
| Single | 469.67 | 488.55 | 4.02% |
| Four-note chord | 788.18 | 863.53 | 9.56% |
| Eight Tide voices | 1319.17 | 1470.36 | 11.46% |

Existing FM6 state already accommodates six operators; D needs no new runtime
array, voice or effect buffer. Activating the extra operator still costs CPU.
Host percentages are not FM-1 utilization. There is no D firmware package
or target timing measurement; integration follows a convincing listening result.

[Metrics/hashes](../build/p0d/listening/measurements.json); run log:
`build/p0d/validation/audition-tests.log`.

## Reproduce

With generated firmware headers and Python/NumPy:

```sh
PYTHON=python3 sh tools/tide-voicing-compare.sh build/p0d/new-listening AD
```

The script refuses to replace existing packs. Recipes remain in the offline
[tide-voicing-patches.h](../tools/tide-voicing-patches.h), which firmware never
includes. No sound is promoted, hardware flashed or user data overwritten.
