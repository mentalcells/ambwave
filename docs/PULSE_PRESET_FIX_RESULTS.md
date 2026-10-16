# PULSE revision: scattered notes and HOME sound selection

2026-10-11 follow-up to the installed prototype `2bb2514`. The user reported
that the slow ordered notes and original preset were not convincing, and that
turning PRESETS made PULSE disappear from ambient layer selection. The user
then chose the role explicitly: “Scattered plucks: randomly choose from my held
notes”. This accepts the playing direction, not final sound tuning.

## Cause and change

HOME PRESETS previously called Felucca's general preset loader. A sound load
clears curated identity and, through panic, held performance inputs. PULSE's
engine parameters and macro ownership were therefore lost; the track still
existed in full UI, but no longer appeared as PULSE. The first test suite
covered explicit full-UI loads but missed this everyday HOME gesture.

On PULSE HOME, PRESETS now cycles three curated PHASE timbres: WARM PLUCK,
SOFT MALLET and BELL PLUCK. The footer and temporary header message identify
the sound. Changes preserve the layer identity, held inputs, transport, arp,
macro intent, mixer level, scale/register and shared FX. MIX still suppresses
preset changes. EDIT retains Felucca's general library and existing load
semantics; loading a general sound there deliberately leaves curated mode.
TIDE and TEXTURE HOME/general-library behavior is unchanged in this fix.

The default arp uses Felucca's RANDOM mode: clocked choices among held pitches,
including possible repeats. It does not invent pitches or randomize event
timing. DENSITY defaults to 50% (half notes: about 0.97 seconds at 124 BPM),
instead of 25% (whole notes: about 1.94 seconds). The complete two-bar through
eighth-note mapping remains available. OCTAVE can put inputs above TIDE; there
is no automatic transposition of the notes the player chooses.

All three sounds use one oscillator, attack 4, zero sustain, release 42 and
unchanged level 72/127. WARM PLUCK is the provisional boot sound, with a rounder
distortion curve and slightly longer decay. The timbres are runtime choices,
not saved scene identities or a replacement general preset format.

| Sound | Existing PHASE wave | SHAPE depth | Envelope depth | Decay |
| --- | --- | --- | --- | --- |
| WARM PLUCK | SAW | 8–68 | 20 | 86 |
| SOFT MALLET | DSIN | 10–65 | 20 | 76 |
| BELL PLUCK | RTRI | 20–85 | 50 | 76 |

MOTION remains slow DRIFT of timbre, DENSITY controls clock divisions and SPACE
controls local sends/dry gain. The profile update is guarded against concurrent
audio parameter reads; it does not call the general sound loader or panic.
No DSP, allocator, storage/package format, loader or recovery changes.

## Validation

The Hasake build and complete `tools/test-macos.sh` suite passed (`ALL HOST
TESTS PASSED`), including 92 unchanged golden renders, CPU budgets,
USB/MIDI/storage/recovery, UI, installer and browser checks. The dedicated
AMBIENT_POC=1 native/WebAssembly comparison passed bit for bit across three
layers and a HOME sound change (220500 frames). Optional DaisySP float-reference
checks were skipped because DaisySP is absent; PHYS stability checks passed.
The isolated TIDE and TEXTURE reference WAV hashes remain unchanged from the
first PULSE build and its accepted TEXTURE reference.

- Hasake build: app 436984 bytes; data+BSS 89788/98304; pool 330208/344064.
  Audio-interrupt target cost stays 39083 against unchanged baseline 37502,
  within the existing 10% allowance. No budgets or golden hashes were updated.
- Physical PRESETS tests reproduce the original failure gesture while running
  and stopped, cycle/wrap both directions, switch away/back, exercise MIX,
  STOP/restart and verify general presets still work in EDIT. No new outgoing
  MIDI messages are emitted by a HOME timbre change.
- The random-note test verifies pitches stay in the held set and all three
  inputs are reached. Exact DENSITY event-count and external-clock checks pass.
- All 48 timbre/macro corners at maximum three-layer levels: peak 20471,
  worst absolute DC 24.2, worst late RMS 43.7. Tails decay after STOP/clear.
- At default settings, random solo warm/mallet/bell RMS: 658.3/566.1/191.6.
  Combined-scene RMS: 7574.7/7601.8/7596.4. These are software measurements,
  not a claim that the user has accepted any of these sounds on hardware.
- HOME/MIX lint covers all three timbres, themes, sizes and styles with no
  layout, clipping, ink, neutrality or alignment findings. The revised HOME
  sound-name footer was also visually inspected.

The existing eight-voice, external MIDI overload, shared FX, full-UI load and
runtime-only scene-state limits remain. Preserving HOME performance while
browsing arbitrary engines is not implemented; the focused HOME sound selector
provides the three curated choices and the full library remains in EDIT.

## Evaluation package

`build/hasake-pulse-r2.fwsc`, 609849 bytes, SHA-256:
`953dcb81a8b8d6125577e26c0779eefe6174a762dca2dc88c669fba5990f0d74`.

Audio comparisons in `build/pulse_demo` include each timbre's random solo,
dense and combined examples. Listening should focus on whether PULSE now adds
useful discrete details, sound selection stays understandable, level is useful
alongside TIDE/TEXTURE, and timbre changes remain comfortable while playing.

## Hardware installation

The revision was installed on 2026-10-12 using the user's explicit FM-1 flash
request earlier in this same conversation. No additional flash permission was
assumed from another chat. `fm1_install.py` completed the write and restart;
a separate identity query confirmed `FM-1_900 [running]` on the Felucca MIDI
port. The revision's random held-note role is user-selected; its three sounds
remain provisional pending listening feedback. The original package remains
available as `build/hasake-pulse.fwsc`.
