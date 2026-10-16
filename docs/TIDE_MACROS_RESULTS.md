# TIDE performance macros

2026-10-15, branch `codex/tide-performance-macros`, based on `f1d4c24`.
The user found the new pad tone better in the emulator, but the knobs too
restrained. For tonight's MVP the model stays intact: strengthen the performance
range, keep TIDE's identity and prepare the public-facing documentation.

## Names and controls

The new variations are now TIDE WARM and TIDE SILK in the HOME heading, normal
track list, footer and sound-change message. Compact mixer/cards use T.WARM and
T.SILK, so both identity and variant fit. The preset order and track/mode
selection gestures are unchanged. Original TIDE is unchanged.

The new variations retain their previous starting positions: SHAPE 50,
MOTION 25, DENSITY 50, SPACE 35. Rendering the same three-note chord for all
three TIDE timbres against `f1d4c24` produces identical samples at defaults.
Combined comparison SHA-256:
`36e64bcb3864f5ff54adbd38814b0833e5a952368442be28178f561f685d8cea`.

| Macro | TIDE WARM | TIDE SILK |
| --- | --- | --- |
| SHAPE | Filter cutoff 32 → 68 → 108; drive 0 → 12 → 22 | FM modulator-level macro -48 → -10 → 63 at the default MOTION |
| MOTION | Retains depth 2/rate 9 at 25%; reaches filter depth 40/rate 38 | Retains depth 1/rate 9 at 25%; higher settings open the harmonic centre and animate it with inverted drift up to depth -64/rate 38 |
| DENSITY | Detune 0 → 7 → 32 cents; oscillator blend 0 → 56 → 64 | Carrier spread macro 0 → 12 → 72 |
| DENSITY chorus | 0 → 16 → 96 | 0 → 16 → 96 |
| SPACE | Dry gain stays full through 50%, falling to 8192 at 100%; reverb reaches 127 and delay 110 | Same |

The arrows mark 0/50/100%, except the explicitly described MOTION anchor.
At SPACE 35 the old reverb send 42 and delay 0 remain. Shared FX bus settings
are unchanged. The existing wet return is mono: DENSITY's audible difference
is beating/thickness/chorus, not a new stereo widening engine. No pitch wobble,
additional voices, new samples or synthesis code was added.

SILK's first soft patch had little FM modulation for a shallow LFO to animate.
The new high MOTION range also opens the modulator-level centre, then sweeps
around it. SHAPE and MOTION deliberately interact in this range, with the
actual engine value clamped to its supported range. All changes remain in the
ambient policy; the FM6 engine and its factory presets are unchanged.

## Validation

The Hasake build succeeds: image 440536 bytes, static RAM 89836/98304,
pool 330208/344064. Target audio interrupt estimate remains 38515 against the
unchanged baseline 37502 (+2.7%, within 10%). No golden hashes or CPU thresholds
were raised.

New held-note renders turn each macro after the chord is already sounding.
They verify harmonic response for SHAPE, time-varying harmonic response for
MOTION, a normalized waveform change beyond gain for DENSITY, and a longer
post-release tail for SPACE. All eight sound/macro comparisons pass. These
are audio-response checks, not a claim of hardware sound approval.

The existing dry/effected health, release-tail, macro-corner and per-track/mode
tests pass. The full `tools/test-macos.sh` suite reports ALL HOST TESTS PASSED:
92 unchanged golden renders, no health/voice-routing failures or CPU entries
over budget, and 184320 HOME/MIX layouts with zero lint/alignment findings.
Storage compatibility, installer and browser checks pass. The explicit Hasake browser build passes its integration checks
and matches native four-track and pad-chord renders bit for bit. The updated
emulator is `build/emu/tide/` at `http://127.0.0.1:8790/tide/`; both variation
names were visually checked and TIDE SILK was left selected.

## Release preparation and rollback

README, QUICK_START, contribution and licensing documents now identify Hasake
and its actual MVP behavior. Original upstream README/contribution documents
are preserved, licence texts remain intact, and the browser/device credits
identify Hasake's source separately from Felucca upstream. Public repository
visibility and emulator hosting remain later release steps; see MVP_RELEASE.md.

`f1d4c24` and its `pads/` emulator are the pre-expansion comparison point.
Earlier hardware rollback material is retained. Hardware installation is
recorded below; listening still needs to confirm the stronger extremes.
Package SHA-256: `a099c15648f435879714ac2c42462579468abeb9981005d7995949ab1a996c61`.

## Hardware installation — 2026-10-15

The user explicitly requested flashing this build. Source `4a9fe20` was rebuilt
with `tools/build-hasake-macos.sh`; its package matches the SHA-256 above.
`tools/fm1_install.py build/felucca.fwsc --yes` verified the package, completed
the write at 100%, restarted the device and reported FM-1_900 running. An
independent `--info` query confirmed FM-1_900 [running] on port Felucca.
The updater/recovery flow was unchanged; no preset, sample or project write
was requested. Installation success does not establish listening approval;
the user still needs to evaluate the sound and macro extremes on the box.
