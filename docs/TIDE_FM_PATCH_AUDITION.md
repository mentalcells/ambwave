# Grounded FM patch audition

2026-10-15. Listening-only experiment, not integrated into Hasake firmware.

The user found both AMB-1 dry reference pads richer and fuller, with the Rhodes
feeling more grounded in the low end. See `AMB1_DRY_REFERENCE.md`. Both earlier
Tide source candidates remain unaccepted; neither is the basis for this audio.

## Focus

Use Felucca's existing FM6 engine without modifying its DSP. A new in-memory
patch has one 1:1 carrier/modulator pair and a quiet, phase-coherent second
harmonic, following the approved Rhodes reference's basic source topology.
There is no added fundamental carrier, unison, detuning, feedback, pitch LFO,
chorus, delay, reverb or stereo width. It does not import AMB-1 DSP.

The main carrier's envelope settles at a stable sustain. The modulator starts
more slowly, then the existing track DRIFT changes its level during the hold.
Nominal phase-modulation depth is about 0.115 turns at full operator envelope;
the quiet second harmonic is about 12.5% of the unmodulated main carrier.
This uses synthesis and operator envelopes, not an EQ treatment of original
Tide. The independent per-voice breathing and stereo placement of the AMB-1
reference are deliberately absent from this comparison.

No product engine selection or macro mapping is changed by this audition.
SHAPE, DENSITY, MOTION and SPACE on the box/emulator retain their existing
behavior. Only after a convincing listening result should this patch be
integrated and validated with those controls. The working firmware sources
still contain the previous, unaccepted harmonic-source experiment; the FM
candidate is applied only inside `tools/tide-fm-audition.c`'s host process.

## Comparison

```sh
PYTHON=.venv/bin/python sh tools/tide-fm-audition.sh
```

Requires the already-generated `build/gen` headers. The script compiles the
same harness against archived `86d2167` for original Tide and the working tree
for the FM patch. The candidate uses FM6 and therefore does not run the rejected
ANALOG source modifications. It does not build/install a firmware package,
write a patch on hardware, or alter presets/projects/samples/backups.

Both takes use D3/A3/F4/C5 (MIDI 50/57/65/72), velocity 100, four simultaneous
notes, identical track/master level and centered pan. They hold for 20 seconds,
then release for 10. Original Tide retains its original oscillator mix/detune;
the candidate adds no detuning. An additional pair uses one D3: original Tide
at DENSITY=0 with filter drift/envelope and oscillator noise removed, and the
new FM source with no detuning. No voice is retriggered during a take.

All spatial sends are explicitly zero before playing, with freshly cleared
shared buffers. The harness verifies each actual bus input and wet return is
zero, and the output channels are identical throughout. Each take must be
audible, bounded, have low DC, finish with no active voices and a silent tail,
and render faster than half real time on the host.

`build/tide_fm_audition/` contains:

- `original-matched.wav` and `candidate-matched.wav`: chord comparison.
- `original-single-matched.wav` and `candidate-single-matched.wav`: one note.
- `AB.wav` and `AB-single.wav`: original 30 seconds, one second silence, candidate.
- Raw WAVs and `levels.json` with matching gains, peaks and pitches.

Matching applies one constant gain per whole take, targeting RMS 6000/32768 over
seconds 8–20; it adds no compression/EQ and preserves the attack/release and
internal dynamics. The source render is raw 16-bit stereo at 44.1 kHz.

## Evidence and decision

Focused render checks pass. Raw held chord RMS is 7165 original / 6681 candidate;
peaks 17822 / 18008, DC -4 / -13.3. Both release to zero and have no active
voices at the end. Four-note render cost is around 0.003–0.009 of host real time
across runs.
The isolated candidate's H2/fundamental amplitude ratio moves about 0.495 to
0.443 over seconds 8–20; this demonstrates sustained timbral response, not a
claim of musical expressiveness or reference matching.

The existing engine passed the last full host/target/browser suite, but those
results do not constitute integrated firmware validation of this new patch.
This patch has only undergone the focused host audition checks so far.

User listening feedback on the matched chord audition: **“It gets closer.”**
This is the first Tide candidate with positive directional feedback on the
requested fullness. Keep this exact patch/audio as the next comparison point.
It is not yet a final richness/expressiveness acceptance or authorization to
install firmware. The remaining tonal gap and live macro behavior still need
evaluation. The follow-up user request was “A bit more mids maybe,” followed
by approval to audition one small revision before integration.
The agent cannot listen directly.

## Midrange revision audition

`PYTHON=.venv/bin/python sh tools/tide-fm-mids-audition.sh` compares the exact
positively heard FM candidate with one changed patch value: the quiet second
harmonic carrier's operator output level goes from 75 to 78. That operator
is about 2.25 dB stronger; this is not a 2.25 dB EQ boost to the total signal.
Its contribution moves from roughly 12.5% to 16.2% of the main carrier.
For this chord its frequency spans about 294–1047 Hz. The main carrier,
modulator, envelopes, drift, pitches and timing are unchanged.

Outputs are kept separately in `build/tide_fm_mids/`:

- `current-matched.wav` and `revised-matched.wav`: same D3/A3/F4/C5 chord.
- `current-single-matched.wav` and `revised-single-matched.wav`: same single D3.
- `AB.wav`, `AB-single.wav`: current 30 seconds, one-second gap, revised.
- `levels.json`: fixed matching gains and output bounds.

The current raw **and** matched chord WAVs compare byte-for-byte equal to the
previously heard FM candidate. Both revisions pass the focused zero-effect-bus
and wet-return checks, identical-channel check, DC/headroom, release and host
cost checks. Raw chord RMS is 6681 / 6670; matching targets 6000 in both, and
matched peaks stay below 16287/32767. Both release to silence with zero voices.
All previous recordings remain in place. No firmware, macro mapping, factory
patch, device or user data is changed by this listening-only revision.

The user heard the subtle midrange revision as **“Sounds pretty similar.”**
Retain the original FM patch as the leading candidate. The user approved a
stronger midrange comparison before choosing a revision for integration.

## Stronger midrange comparison — 2026-10-16

`PYTHON=.venv/bin/python sh tools/tide-fm-strong-mids-audition.sh` renders the
original FM candidate against a larger change to the same source component:
OP3 output level 75 to 87. This is about +9 dB for the second-harmonic operator,
moving its contribution from roughly 12.5% to 35.4% of the main carrier; it is
not a +9 dB boost to the whole sound. All other patch values, motion, notes,
timing and dry routing remain unchanged. No processing stage is added.

Files are in `build/tide_fm_strong_mids/`, with `current-matched.wav` and
`revised-matched.wav` for the chord, matching `*-single-matched.wav` files,
and `AB.wav` / `AB-single.wav`. Previous comparisons are preserved.

The current raw and matched chord files again compare byte-for-byte equal to
the positively heard FM patch. Focused dry-routing, mono-channel equality,
signal health, completed-release and host-cost checks pass for both takes.
Raw chord RMS is 6681 / 6660; fixed matching produces RMS 6000 for both with
peaks 16172 / 16567, below clipping. Both finish with silence and no voices.

Listening preference for this stronger revision is pending. The patch remains
host-only; firmware integration, four-macro validation and emulator testing
follow selection of a convincing source. No hardware installation is authorized.

## Integrated candidate — 2026-10-16

The stronger midrange variant also sounded similar to the user, who asked to
stop tonal refinements. Following explicit approval, original TIDE now uses
the first positively heard FM patch (OP3 OL75). WARM and SILK retain their
previous patches. The rejected ANALOG experiments are removed from the active
oscillator code; their source, package and recordings remain under build/.

The four performance controls remain:

| Control | Default | Meaning for original TIDE |
|---|---:|---|
| Shape | 50% | FM modulator level, E2 -16 / 0 / +12 at 0 / 50 / 100% |
| Motion | 25% | Existing smooth DRIFT into FM index; no pitch modulation |
| Density | 50% | Quiet second-harmonic carrier gain, -12 / 0 / +12 dB relative to the heard patch |
| Space | 50% | Existing delay/reverb sends; 0% has zero chorus, delay and reverb sends |

Original TIDE never sends to chorus, at any macro setting. Density changes
held-note harmonic weight through the existing per-block FM gain ramp. This
requires one AMBIENT_POC-only runtime field per FM voice (96 bytes total),
without changing patch, project or preset storage formats. The silent envelope
floor is preserved so positive gain cannot keep a released note alive. At the
defaults this runtime offset is exactly zero.

`build/tide_fm_demo/default.wav` is byte-for-byte identical to the previously
heard `build/tide_fm_audition/candidate.wav`, including its 20-second sustain
and 10-second release. This proves preservation, not a new listening verdict.
The user previously heard that source as “It gets closer”; final acceptance
of the playable macro ranges remains a listening decision.

The playable local emulator is served at http://127.0.0.1:8790/tide-fm/ .
Click Switch on if needed. HOME starts at SHAPE50 / MOTION25 / DENSITY50 /
SPACE50. Drag KNOB4 down to SPACE0 before playing to hear it dry; click keys
to latch up to four notes, and click them again to release. Turn KNOB1–3
while a note is held to judge sustained timbre and response. The tab prepared
in this session is already at SPACE0. No hardware was flashed.

Validation of the integrated source:

- `tools/build-hasake-macos.sh`: succeeds; image 441312 bytes, RAM data+BSS
  89932/98304 bytes, pool 330208/344064 bytes; unchanged loader/package format.
- `EM_CACHE="$PWD/.toolchains/emscripten-cache" tools/test-macos.sh`:
  ALL HOST TESTS PASSED, including 92 unchanged golden renders, target loop
  budgets, storage, UI, four-layer macro corners and voice pressure. Optional
  DaisySP reference tests were skipped because its checkout is absent.
- Dedicated AMBIENT_POC=1 emulator/native comparisons pass, including the
  original TIDE chord, four-layer mix, and WARM/SILK chords bit for bit.
- Final `tests/tide_fm_test.c` rerun passes both default and maximum Density:
  zero spatial sends/returns, identical channels, bounded output/DC, silence
  and zero voices after release. Density changes the rendered gain of the
  held harmonic carrier by the intended ratio without changing its frequency.
  All three Tide variants stay dry after either ordering of Space/Density edits.
- Browser UI boots on TIDE; its four macros are visible. The prepared tab was
  set to Space0 through the actual knob and left open for listening.
- Hardware package: `build/felucca.fwsc`, SHA-256
  `4b863afc36c630e15fa863d6abfec235ca4e907d8ed40c40791feb9fc29dbfc8`.
  Building this file does not authorize installation; no hardware was flashed.

## Hardware installation — 2026-10-16

The user explicitly requested “let's flash” in this conversation. The connected
Felucca MIDI device identified as running FM-1_900. Installed the above verified
package with `.venv/bin/python tools/fm1_install.py build/felucca.fwsc --port Felucca --yes`.
The installer completed writing, restarted the device, verified FM-1_900, and
exited successfully. Hardware listening remains for the user; the identity
check confirms restart, not sound quality. No user-data erase was requested.
