# Independent sound slots

2026-10-14. Topic branch `codex/ambient-sound-slots`, based on merged
`ambient-prototype` at `a0c4e5f`.

## Accepted change

The user clarified that CLOUD should be another available layer, and that any
track should be able to select TIDE, TEXTURE, CLOUD or PULSE. Layers and sounds
are now separate runtime concepts. This supersedes the earlier fixed-slot
CLOUD/TEXTURE arrangement.

Boot has four silent, playable layers: 1 TIDE, 2 TEXTURE, 3 PULSE, 4 CLOUD.
ALGORITHM selects all four. Each layer has its own sound identity, macro values,
held notes, PULSE timbre/speed/gate state, dry perspective and mixer slew.
Duplicate sounds on different layers are supported.

PRESETS on any curated HOME cycles the same six entries:
TIDE → TEXTURE → CLOUD → PULSE warm pluck → PULSE soft mallet → PULSE bell pluck.
The HOME screen, layer level, layer mute and sequence are retained. Changing a
sound clears that layer's physical hold and pending/generated notes, balances
outgoing physical MIDI note-offs, and resets its macros/playing behavior. Other
layers and global FX settings are retained. Sound selection while MIX is held
is suppressed. Full EDIT pages retain Felucca's general engine/preset library;
loading an arbitrary sound there or loading a project leaves curated mode for
that track, as before. This is separate from scene recall/persistence.

TIDE uses up to four held pitches; TEXTURE and CLOUD use one toggle-held pitch;
PULSE queues up to four pitches and scatters notes while transport runs. STOP
releases every PULSE layer, including pitch zero, while retaining queued input.
MIDI input ownership and the shared eight-voice allocator are reused.

## Controls

- Four HOME knobs: SHAPE / MOTION / DENSITY / SPACE of the selected sound.
- HOLD HOME + knobs 1–4: levels of layers 1–4.
- HOLD HOME + the first four black keys: independent layer mutes; tails finish.
- HOLD HOME + SELECT: selected PULSE layer speed (half, quarter, eighth,
  sixteenth). SELECT normally remains master tempo.
- FX + OCT−: clear held input on the selected layer.

HOME/MIX shows all four assigned sounds. PULSE rows show RUN/STOP and the
selected division. The footer identifies the PULSE timbre.

## Shared effects and sound compatibility

SPACE now changes local sends/dry level for every sound. Shared bus settings
start at the accepted TIDE midpoint (delay 1/8, feedback 54, mix 82, reverb size
108/damping 76). Changing SPACE on a TIDE assigned to any slot no longer changes
other layers' room/delay. Global FX editing remains available in the existing
pages. This deliberately changes TIDE's SPACE extremes; its default engine,
envelope, oscillator mix, chorus and sends are retained. The SPACE render check
isolates the wet tail from TIDE's long envelope release to verify local sends.
No engine DSP, storage format, loader, bootloader or package format is changed.
No golden engine audio hashes are updated.

## Software evidence

- Firmware builds via `tools/build-hasake-macos.sh` / `./build.sh`: image
  437520 bytes; static RAM 89820/98304; pool 330208/344064.
- Focused slot tests cover all six sound entries on all four slots through the
  real HOME PRESETS gesture, outgoing MIDI balance, retained mix/mutes/sequence,
  local macro independence, duplicate CLOUD/PULSE ownership and all-track STOP.
- Sixteen four-layer macro corners retain four sustained TIDE gates plus
  TEXTURE and CLOUD while PULSE runs within the shared eight-voice budget;
  output is audible/bounded with bounded DC. Host cost is about 0.003 of real
  time on this Mac. This is a regression measurement, not device CPU approval.
- HOME/MIX render lint covers sound rotations across all four slots, PULSE
  variants, all 16 mute masks, themes/styles and card sizes: 61,440 layouts,
  zero lint or alignment findings.

The first full run passed its functional/audio/storage/UI checks and caught a
26% increase in the static audio-ISR loop estimate. In the layered build the
bounded arpeggiator scheduler now stays outside the compiler-inlined ISR body;
transport slot scans are also kept out of that body. The target estimate is
37,551 against the unchanged 37,502 baseline (+0.13%, allowed +10%). No CPU
budget or golden hash was raised. The full suite was rerun after the fix and ended with `ALL HOST TESTS PASSED`.
All 92 golden renders remain unchanged, with zero health/voice failures and
zero host CPU entries over budget. The optional DaisySP reference check was
skipped because its external checkout is absent.

The AMBIENT_POC=1 emulator passed boot, all six HOME sound entries on all four
slots, fourth-layer hold/mute, selected-PULSE speed and persistence checks. Its
four-layer performance render (variation, speed, mute/resume and STOP/restart)
is byte-identical to the native build. The visible browser was also checked:
track 1 changed from TIDE to TEXTURE while retaining HOME/macros, then returned
to TIDE; layer 4 is independently selectable as CLOUD.

The runnable browser bundle is `build/emu/slots/`, served by the existing local
server at `http://127.0.0.1:8790/slots/`. It is kept separate from the upstream
compatibility emulator produced by the full test runner.

## Limits

Hardware listening and on-device four-layer CPU margin remain unverified.
The shared eight-voice budget still applies: arbitrary combinations of multiple
four-note TIDE layers, external MIDI and long release tails can steal voices.
The default four-sound arrangement passed the focused software pressure check.
Sound selection/macro identity remain runtime-only; project data is compatible,
but project loading does not implement Hasake scene recall. No device was flashed.
