# Independent playing modes

2026-10-14. Branch `codex/ambient-playing-modes`, based on the committed
sound-slot prototype `d94a424`. The user authorized trying a separate feature
branch after finding that CLOUD needed deliberate note sequencing.

## Model

Four tracks, each with an independently selectable sound and playing mode.
TIDE/TEXTURE/CLOUD/PULSE are sound families, not fixed track roles. Startup
remains TIDE HOLD, TEXTURE HOLD, PULSE SCATTER, CLOUD HOLD. PULSE retains its
existing name for its three pluck timbres.

HOLD toggles sustained input: up to four pitches for TIDE/plucks, one register
for TEXTURE/CLOUD. SCATTER toggles up to four pitches on any sound and uses
Felucca's random arp while PLAY runs. SEQUENCE uses the existing saved step
pattern and sequencer while PLAY runs; physical input is momentary for audition
and step entry. STOP releases generated notes and retains input/pattern data.
A HOLD/SCATTER track does not also play its stored sequence.

PRESETS on HOME changes only the sound: it retains playing mode, scatter speed,
pattern, level and mute. As previously, a sound change clears held/generated
notes and resets sound macros. Changing playing mode also clears live input and
releases local notes (including the selected track's external MIDI ownership),
but retains the written pattern. Other tracks keep their own mode and material.

## Try CLOUD

1. ALGORITHM selects track 4 (CLOUD).
2. HOLD: tap a key to sustain it; tap again to stop. Try the four macros.
3. HOME + ARP cycles HOLD → SCATTER → SEQUENCE → HOLD. This is a HOME gesture;
   normal ARP pages remain available when HOME is not held.
4. SCATTER: tap several pitches, then PLAY. STOP pauses generation and retains
   the pitch set. CLOUD's DENSITY still controls grains; scatter ticks play at
   full probability. PULSE retains its accepted density/probability mapping.
5. SEQ from curated HOME selects SEQUENCE and opens the existing STEP editor at
   step 1. An empty track at the default length starts with eight half-note
   steps. Tap/release successive notes; releasing advances the cursor. EDIT
   clears the current step and advances, allowing rests. SEQ again opens
   PATTERN, where LEN/division/gate remain editable. Existing nonempty patterns
   and custom lengths are retained.
6. HOME returns to the four macros. PLAY repeats the written phrase. Turn
   PRESETS to audition another sound on the same phrase without leaving HOME.

HOME + SELECT adjusts the selected SCATTER or SEQUENCE speed through half,
quarter, eighth and sixteenth notes. Sequence changes start from the actual
stored division. HOLD HOME + knobs still mixes four tracks; HOME + the first
four black keys still mutes tracks. HOME shows each track's sound and mode.

In the browser, Shift-click HOME holds it; click ARP, then Shift-click HOME
again to release. The normal piano keys and SEQ/HOME/PLAY buttons work directly.
The emulator bundle is `build/emu/modes/`, served at
`http://127.0.0.1:8790/modes/`; the prior sound-slot bundle remains in `slots/`.

## Sound and resource policy

No engine DSP is added. Clocked TIDE/TEXTURE/CLOUD use shorter existing ADSR
attack/release settings (24/48) to make individual notes audible and limit release
pressure; HOLD retains its accepted envelopes. PULSE's existing scatter tuning
is retained. PULSE DENSITY in HOLD/SEQUENCE instead adds existing PHASE detune
and chorus width; its SCATTER mode keeps the accepted hit-probability behavior.
SHAPE/MOTION/SPACE mappings and shared FX ownership remain as in sound slots.

The one-register chord limit now follows the sound in HOLD mode, rather than
track 2. SCATTER accepts a full pitch set regardless of the sound or slot.

## Validation

- Firmware package builds via `tools/build-hasake-macos.sh` / `./build.sh`:
  image 438324 bytes, static RAM 89820/98304, pool 330208/344064.
- Full `tools/test-macos.sh` / `sh tests/run_tests.sh`: ALL HOST TESTS PASSED.
  All 92 golden engine renders are unchanged, zero health/voice failures,
  zero host CPU entries over budget. CPU baselines/hashes were not raised.
- Audio interrupt target estimate 37815 against unchanged baseline 37502
  (+0.83%, within the existing +10% limit).
- Focused tests cover physical HOME/ARP and SEQ gestures, eight-note CLOUD step
  entry without preview latching, exact pitch-order playback, STOP, mode/sound
  independence on every track, preserved patterns and actual-division speed.
  A written CLOUD phrase with four-note TIDE, TEXTURE and PULSE remains audible,
  bounded and within the shared voice budget.
- HOME/MIX mode/sound/mute/theme/style render sweep: 184320 layouts with zero
  lint/alignment findings.
- AMBIENT_POC=1 browser tests pass physical eight-note phrase entry, retained
  phrase/mode across sound changes, audible sequenced CLOUD and HOME/ARP return
  to HOLD. Four-track native and WebAssembly performance renders match exactly.

Hardware envelope/sound tuning and on-device CPU margin remain unverified.
The shared eight-voice limit still applies. Mode and curated identity are runtime
state; no scene recall or storage format change is added. Underlying sequence
storage remains Felucca-compatible. Hardware installation is recorded below.


## Hardware installation (2026-10-14)

The user explicitly requested flashing the connected FM-1 in this conversation.
Implementation `4c42c71` was rebuilt with `tools/build-hasake-macos.sh` and
installed through the unchanged `tools/fm1_install.py` flow. Package SHA-256:
`4018e1c51638423740af3010eac3adc367d7abb690a5ac435ea366cc5657bb95`.

The installer validated the package, completed the write at 100%, restarted,
and reported `done: the FM-1 runs FM-1_900`. An independent `--info` query then
confirmed `FM-1_900 [running]` on port `Felucca`. No user presets, samples,
projects or backups were deliberately written by this operation. Playing-mode
workflow, sound tuning and on-device load still await user listening feedback;
successful installation does not establish those approvals.
