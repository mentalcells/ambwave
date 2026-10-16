# PULSE controls and direct layer mutes

2026-10-12 follow-up to the installed `462be02` revision. The user accepted it
as a beginning, found the four knobs too subtle, and approved a more audible
macro design, separate speed/density, and quick HOME mutes. MOTION's accepted
role is variation between individual plucks. This approves the controls and
implementation direction; the new sound still needs hardware listening.

## Playing controls

ALGORITHM selects TIDE / TEXTURE / PULSE. PRESETS on PULSE HOME still cycles
WARM PLUCK / SOFT MALLET / BELL PLUCK without losing notes or controls.
The four normal PULSE knobs now have these roles:

| Knob | Behavior |
| --- | --- |
| SHAPE | Soft, damped to bright, ringing. PHASE distortion and its envelope depth rise; attack shortens, decay and release lengthen. |
| MOTION | Per-hit tone and loudness variation, plus a restrained stereo spread; no pitch modulation or added notes. Zero disables all three matrix amounts. |
| DENSITY | Probability of a pluck on each clock tick. At zero, 4/128 ticks; at 50%, 66/128; at 100%, every tick. Every integer detent changes the probability threshold. |
| SPACE | Dry/close to echoes and distant trails using local sends and dry gain. Delay sends start above 35%; above 50% the dry signal falls to 25% at maximum. |

Hold HOME: knobs 1–3 adjust TIDE / TEXTURE / PULSE levels; knob 4 is SPEED,
with half, quarter, eighth and sixteenth-note divisions. It works from any
curated layer's HOME, so PULSE can be paced without changing layer focus.
Speed clamps at the endpoints; SELECT remains the master tempo control.
The default is eighth notes at 50% density: around two hits/second on average
at 124 BPM, with random gaps. Timing stays on the clock and pitches remain
random choices from held notes, with repeats allowed.

SHAPE uses existing PHASE waves SAW / DSIN / RTRI, one oscillator throughout.
Distortion ranges are 2–92 / 4–100 / 8–104. Attack ranges 20–2, decay 52–90,
release 24–58, sustain remains zero. Envelope distortion ranges 6–28 for warm
and mallet, 6–40 for bell. MOTION reuses the existing RAND modulation source
with SHP / AMP / PAN destinations, maximum amounts 28 / 32 / 18. Tone and gain
are per voice; pan follows the latest hit, so overlapping tails share its pan.
No new DSP, voices, allocators or modulation sources were introduced.

## Quick mute

Hold HOME and tap the first three black keys (F#3 / G#3 / A#3) to toggle
TIDE / TEXTURE / PULSE mute. Release HOME: the mute stays. Repeat to unmute.
The same keys light when muted and breathe when available while HOME is held.
MUTE appears in the layer's HOME status and its MIX card, while the card's
level bar retains the saved runtime level. HOME captures its keys at the audio
boundary, including a simultaneous HOME/key press, without adding held notes
or emitting MIDI. Other HOME-held keys are consumed without playing notes.

These runtime mutes reuse the existing FX gain ramp before dry/FX sends.
Held notes, mixer values and PULSE timing continue; existing wet tails finish
naturally and an unmuted drone resumes without another key press. Muting an
idle layer initializes its gain to zero so its first future note cannot leak
through. FX momentary mutes and FX LATCH still work independently. A general
sound load clears that track's HOME mute; curated PULSE timbre changes retain
it. Mutes consume the same running voices/CPU and are not project storage.
STOP still releases PULSE's generated gate while preserving its input notes;
FX + OCT- still clears the selected layer's held notes.

## Validation

Focused PULSE tests pass: four speed rates, probability at 0/25/50/75/100%,
single-detent response, random held-pitch selection, independent input/MIDI
ownership, STOP/restart/external clock, timbre changes preserving speed/mute,
muting already-running and idle layers, no MIDI leakage, and fastest-rate
voice pressure with four TIDE voices plus TEXTURE held.

Measured dry repeated identical pitches: MOTION zero has 0.4% RMS variation
and no stereo difference; MOTION maximum has 24.3% RMS variation and 13.9%
normalized stereo difference. All three SHAPE endpoint renders show clearly
longer ringing tails. All 48 timbre/macro corners at sixteenth-note speed and
maximum three-layer levels: peak 21885, worst absolute DC 30.2, worst late RMS
43.7, no clipping or stuck tails. These are software evidence, not subjective
hardware acceptance.

All 5,760 focused HOME/MIX screens pass layout, neutrality and alignment lint
across every palette, style, size, selected layer, timbre and mute combination.
The new mute/speed layout was visually inspected. Isolated TIDE and TEXTURE
WAV hashes match the accepted references. Target audio IRQ cost is 40782
against the unchanged 37502 baseline, within its 10% allowance. App 438112 B,
RAM data+BSS 89788/98304 B and pool 330208/344064 B. Budgets/goldens unchanged.

The final Hasake build and complete `tools/test-macos.sh` suite pass (`ALL HOST
TESTS PASSED`), with 92 unchanged golden renders, CPU budgets, storage/USB/MIDI,
UI, installer and browser checks. The AMBIENT_POC=1 native/WebAssembly comparison
also passes: 220500 frames across three layers, timbre and register changes,
per-hit variation, speed, mute/resume, STOP/restart and clear; the heavy song
matches over 88200 frames. Optional DaisySP reference checks were skipped
because DaisySP is absent; PHYS stability tests passed.

## Package and installation

`build/hasake-pulse-r3.fwsc`, 609849 bytes, SHA-256:
`acd249b27178f2309aa666cab65fae7335c9a4926151b3d3b53ef17d3f2ed25f`.
Build and focused evidence is retained in `build/pulse_r3_validation/`.
Prior R1/R2 packages remain available; no user presets, samples, projects or
backups are written by this change. Loader, recovery and package format are
unchanged. Eight-voice external-MIDI overload and shared FX bus limits remain.

Installed on 2026-10-12 using the user's explicit FM-1 flash approval in this
conversation. `fm1_install.py` completed package validation, write and restart.
A separate identity query confirmed `FM-1_900 [running]` on the Felucca port.
The previous R1/R2 packages are retained. Final sound acceptance awaits the
user's listening feedback.
