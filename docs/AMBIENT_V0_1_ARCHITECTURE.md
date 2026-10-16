# FM-1 Ambient OS: Felucca architecture and v0.1 plan

Status: design investigation only. This document does not change firmware behavior.

## Recommendation

Build v0.1 as a thin ambient mode around Felucca, not as a new synthesizer firmware.
Felucca already has the required real-time audio path, a suitable polyphonic analog
engine, long envelopes, shared delay and reverb, physical-key routing, four home-page
knobs, and a safe way to start and stop notes. The proof of concept only needs:

1. one ambient boot configuration on track 1;
2. a physical-key latch policy for that track;
3. a `SHAPE` macro which initially controls the analog filter cutoff; and
4. a simplified home screen which exposes the ambient concept rather than Felucca's
   workstation terminology.

Use the existing **ANALOG / SOFT PAD** sound as the first source. It is the lowest-risk
choice because its first home knob already controls cutoff, it supports sustained
polyphony, and it does not depend on sample data or a granular seek index. GRAIN is an
excellent candidate for the later TEXTURE role, but using it in the first proof would
test granular behavior and the ambient interaction model at the same time.

Do not remove the sequencer, drums, preset browser, sample uploader, or other engines
for v0.1. Leave them compiled and simply keep them out of the ambient boot path and
performance screen. This minimizes the fork and retains Felucca's recovery paths and
test coverage.

## Current architecture at a glance

Felucca is assembled as one firmware compilation unit in
`firmware/src/felucca.c`. Its real-time flow is:

```text
panel keys / USB MIDI / TRS MIDI / sequencer / arpeggiator
                         |
                         v
                input_on / input_off
                         |
                         v
                trk_note_on / trk_note_off
                         |
                         v
           shared voice allocation + ADSR + modulation
                         |
                         v
              selected engine render callback
                         |
                         v
        track distortion -> slicer -> level/pan/sends
                         |
                         v
             shared chorus + delay + reverb
                         |
                         v
        performance FX -> limiter -> DAC / USB audio
```

The separation is useful for Ambient OS: a latch or macro does not need to know how an
oscillator, grain, or FM operator works. It can continue to address notes and existing
parameters through the common track interface.

## Synthesis engines

The engine contract is `engine_t` in `firmware/src/core.h`. An engine supplies:

- a name and two edit-page titles;
- eight engine-specific parameter descriptors (`P_E0` through `P_E7`);
- factory presets;
- four parameters for the home knobs;
- optional polyphony and state-retention limits; and
- note-on, render, envelope, block, and description callbacks as needed.

The registry in `firmware/src/engines.c` is append-only because projects, user presets,
and the editor protocol store engine numbers. It currently contains ANALOG, PHASE,
LOFI, SAMPLE, VOICE, TRIO, WHEEL, GRAIN, PHYS, DRUM, NOISE, FM6, and optionally
SLICE. Engine number 1 remains reserved for the retired DIGITAL engine and is converted
to FM6 when DIGITAL support is not built. Ambient work must not reorder this table or
the parameter IDs.

Engine changes are already safe while audio is running. The UI requests a new engine,
`voice.c` fades the old one over four audio blocks, switches at a block boundary, and
then starts any notes queued during the transition.

Existing engines relevant to the longer-term design are:

| Ambient use | Existing component | Useful behavior |
| --- | --- | --- |
| First DRONE | `eng_analog.c` | Two oscillators, noise, drive, resonant low-pass, polyphony, pad/string presets. Home knob 1 is cutoff. |
| TEXTURE | `eng_grain.c` | Built-in and user sample sources; position, size, density, pitch, spread, randomness, and tone; three voices and twelve grains per track. Spread zero already behaves like a fixed/frozen source position. |
| Air/noise layer | `eng_noise.c` | Wind/rain-like material and an existing slow `DRFT` parameter. |
| Resonant drone | `eng_phys.c` | Physical models and an existing `DRONE STRING` preset. |
| Harmonic/metallic voice | `eng_fm6.c` | Six-operator FM, six-voice cap, its own operator envelopes. |

For v0.1, none of these DSP engines needs modification.

## Tracks and voices

Felucca has four tracks/parts (`NPART`/`NTRK`) and eight voice slots per track. The
instrument also enforces a shared maximum of eight simultaneously sounding voices
across all tracks. Individual engines may impose a lower cap; GRAIN and PHYS cap a
track at three voices, and FM6 at six.

`track_t` in `firmware/src/core.h` owns:

- all common and engine parameters;
- the current/requested engine and preset;
- voice slots and LFO state;
- held notes and arpeggiator runtime;
- 64 sequencer steps and recording state;
- mono/legato/unison state;
- mixer, distortion, and tail state; and
- modulation-matrix controller state.

`voice.c` implements the common polyphonic, mono, legato, and unison modes. Voice
stealing prefers released tails and extra unison voices before held musical notes. It
fades a stolen voice for one block to avoid a hard discontinuity. Engines receive the
same `voice_t` structure and do not know whether a note came from the panel, MIDI, the
arpeggiator, or the sequencer.

At normal boot, `felucca_init()` initializes four sounds from `TRK_DEF` in
`firmware/src/engines.c`: ANALOG ACID, FM6 PAD, LOFI PULSE LEAD, and DRUM KIT. The
patterns are empty. Ambient v0.1 should replace only this boot selection: configure
track 1 as the drone and mute tracks 2-4. It does not need a new track type or new voice
allocator.

The eight-voice global limit matters later. A latched drone chord should initially be
limited to four notes so it leaves headroom for future TEXTURE and VOICE roles.

## Knobs and buttons

`firmware/src/panel.c` maps the physical controls. The encoders are SELECT,
ALGORITHM, PRESETS, and KNOB 1-4. The buttons are FX, SCL, ENV, LFO, EDIT, GLO, HOME,
SAVE, ARP, SEQ, PLAY, REC, OCT-, and OCT+. Panel calibration and debouncing are already
handled below the UI.

The parameter system in `params.c` is table driven. Each page specifies a family,
scope, graph, and four parameter IDs. `ui_input.c` turns encoder detents into bounded
parameter changes and records automation where appropriate. On the normal HOME screen,
`home_param()` reads the selected engine's four-entry `knob` map. ANALOG currently maps
those knobs to cutoff, resonance, attack, and release.

Buttons support taps, holds, and lockable quick layers. The FX layer is especially
relevant to the future design: while it is held or locked, the keyboard triggers
performance effects and the four knobs control FILTER, CRUSH, THROW, and DEPTH. That
system proves Felucca can temporarily give controls a musical-intent mapping, but its
current labels and mappings are not the Ambient OS macro model.

For v0.1, route KNOB 1 to the existing ANALOG cutoff parameter and label it `SHAPE`.
This is a direct, musically useful dark-to-bright sweep and needs no new DSP. Leave the
other three macros visibly unavailable or at fixed scene defaults until they are
implemented; do not present dummy controls that appear active.

## Display and UI

The UI targets the 240 x 240 display. `ui.c` holds navigation and edit state,
`ui_draw.c` draws headers and four parameter cards, `ui_graph.c` draws page-specific
graphs, and `ui_layer.c` draws the quick-layer maps. Fonts, icons, palette handling,
messages, menus, value formatting, dirty/forced redraws, and the four-column control
layout can all be reused.

The current HOME page is generic: it says `HOME` and draws the labels and values of the
selected engine's four home parameters. Other pages expose envelopes, LFO, modulation,
effects, scales, engines, mixer, presets, arpeggiator, sequencer, and projects.

The smallest ambient UI change is a home-page presentation override, not a replacement
UI framework. It should show:

- scene name, initially `DEEP WATER` (working name);
- latch state and the number of latched notes;
- `SHAPE` above knob 1; and
- inactive placeholders for MOVE, TEXTURE, and SPACE until each becomes real.

Normal menus and diagnostic/recovery features can remain reachable through an explicit
hold gesture during development. Ordinary power-on should land on this screen without
requiring an engine or preset selection.

## Effects

`firmware/src/fx.c` supplies a per-track distortion insert and slicer, then per-track
sends to shared mono chorus, tempo delay, and reverb buses. The dry tracks are stereo
through level and pan. The global reverb offers ROOM and SPRING models. The master path
adds DC blocking, optional speaker low-cut/bass processing, a peak limiter, and soft
clipping.

`firmware/src/perform.c` adds temporary master effects for the FX quick layer:
repeat, reverse, low-pass, high-pass, tape stop, freeze, octave up/down, crush, and send
throw. Its FREEZE captures about 186 ms into a shared performance buffer and grains it;
it is a master performance effect, not the same thing as holding a GRAIN engine source
position. It may be reusable for the later FREEZE gesture, but the musical semantics
should be evaluated before naming it as the final feature.

For v0.1, reuse delay and reverb unchanged. The boot scene only needs to set the track's
existing delay/reverb sends and the existing global delay/reverb parameters. The later
SPACE macro can fan one knob into several of these values, but that mapping is outside
the first one-knob proof.

## Presets and scenes

Factory presets live with each engine. `apply_preset_to()` in `ui.c` restores the
engine's eight parameters, ADSR, filter-envelope amount, mono/poly choice, and four
effect values. It intentionally leaves track-level settings such as mixer state,
arpeggiator settings, scale, pattern timing, and slicer configuration alone. A preset
may suggest a factory pattern, but loading the sound does not load that pattern.

There are also 32 user-preset slots in `upreset.c`. A record stores the engine, name,
instrument parameters, and an optional 16-step pattern; FM6 patch data is stored beside
it. Projects preserve broader instrument state.

Do not add a scene storage format in v0.1. Represent the sole scene as a small boot
configuration applied after Felucca's normal defaults. This is enough to test the
instrument feel without committing to how future scenes will store three roles, macro
mappings, samples, and effect state. Existing preset and project formats can remain
byte-compatible and unchanged.

## Samples

Built-in CC0 samples under `assets/samples-cc0` are converted by
`tools/gen_samples.py` into compact flash tables used by SAMPLE, GRAIN, and SLICE.
SAMPLE and GRAIN share keyboard zones and ADPCM decoding infrastructure. Missing sample
data has a safe sine fallback.

Felucca also supports three user sample slots in flash. The web editor uploads them
through commands implemented in `editor.c`; the firmware validates headers, zones,
lengths, and CRCs before publishing a slot to the engines.

The v0.1 analog drone uses none of this. Keep the built-in sample data and upload code
unchanged, and do not expose a user-sample workflow. The same infrastructure can later
provide a built-in source for the TEXTURE role without designing any new storage.

## Note latching today

All live input converges in `seq.c`. Physical key presses are mapped to notes/chords,
then `input_on()` either sends the note to the arpeggiator or calls `trk_note_on()`.
Key release reaches `input_off()` and ultimately `trk_note_off()`. USB and TRS MIDI use
the same note endpoint and include ownership checks to avoid releasing a note still held
by another source.

Felucca has two existing features called or behaving like latch:

1. **Arpeggiator HOLD.** `P_AHOLD` retains the arpeggiator's held-note list after keys
   are released. When no key is physically down, the next played chord replaces the
   retained chord. The arpeggiator still gates and retriggers notes, including in REPEAT
   mode, so it is not a continuous drone latch.
2. **FX LATCH.** Performance effects in the FX layer can be toggled and left active.
   This latches effects, not musical notes.

`track_t` contains a `latched` byte labelled for HOLD, but it is not read or written by
the current implementation. The active arpeggiator latch state is instead represented
by `held[]`, `nheld`, `arp_phys`, and `P_AHOLD`.

The requested LATCH differs from both existing features: pressing a keyboard note
should add it to or remove it from a sustained drone, and releasing the physical key
must not release the sound. Reusing ARP HOLD would be fewer lines but would give the
wrong feel because it replaces a chord and periodically retriggers it.

The smallest correct change is a narrow physical-key latch adapter in `seq.c`:

- on key-down, toggle each mapped note in a small per-track latched-note set;
- call the existing `trk_note_on()` when adding and `trk_note_off()` when removing;
- ignore the corresponding physical key-up for the latched sound;
- keep the physical key LED steadily lit while its note remains latched, with a
  quick layer's temporary key map taking visual priority while that layer is open;
- cap the initial set at four notes;
- clear it through the existing all-notes-off/panic path, on scene reinitialization,
  and when latch mode is disabled; and
- keep USB/TRS MIDI in normal momentary mode for v0.1 so MIDI ownership behavior does
  not change accidentally.

For the first latch milestone, the clear gesture is **hold FX and press OCT−**. In an
`AMBIENT_POC` build it performs an all-notes-off on the ambient track (and retains the
existing FX-all-off meaning), so it is available without starting the transport or
entering a menu.

This is input policy only. It requires no engine, envelope, voice allocator, or audio
mixer changes.

## Smallest v0.1 change set

### Required

1. **Ambient boot configuration.** After normal initialization, configure track 1 as
   ANALOG using SOFT PAD as the starting point, adjust its sustain/release and existing
   sends for a stable drone, select it, mute tracks 2-4, leave all patterns empty, and
   go directly to the ambient home screen.
2. **Physical-key drone latch.** Add the toggle behavior described above, limited to
   track 1 and four notes. Provide one obvious clear/all-off gesture.
3. **One real macro.** Make KNOB 1 edit ANALOG cutoff over a curated range and display
   it as `SHAPE`. The mapping can initially be direct; only add a curve if listening
   tests show the useful range is bunched at one end.
4. **Minimal ambient home presentation.** Show the scene, latch status, and macro
   labels using the existing renderer. Keep a development escape to the normal UI.
5. **Tests and firmware build.** Add host tests for boot state, latch add/remove/clear,
   the four-note cap, and SHAPE bounds. Run the existing host suite and the established
   FM-1 firmware build. Do not flash hardware without explicit approval.

### Explicitly deferred

- TEXTURE and VOICE roles as user-facing track concepts;
- MOVE, TEXTURE, and SPACE macro mappings;
- DRIFT and final FREEZE behavior;
- multiple scenes or scene storage;
- sequencer and drum removal;
- user samples;
- complex menus and macro editing;
- a rewrite of MIDI behavior; and
- changes to engine IDs, parameter IDs, or stored formats.

## Components reusable unchanged

| Area | Reuse unchanged for v0.1 |
| --- | --- |
| Hardware and boot | FM-1 HAL, input scanning/calibration, display driver, audio DMA, loader, package, OTA/recovery paths |
| Audio | Block scheduler, DAC/USB audio conversion, overload protection, limiter, DC blocker |
| Synthesis | Engine registry and callbacks; ANALOG DSP; all other engines remain compiled but unused by the ambient boot scene |
| Notes and voices | `trk_note_on/off`, common ADSR, poly/mono allocator, voice stealing, engine-switch fade, all-notes-off |
| Modulation | LFO and modulation matrix; not required initially but available for MOVE/DRIFT later |
| Effects | Track distortion/sends, shared chorus/delay/reverb, master protection |
| UI foundation | Panel decoding, encoder stepping, fonts, icons, palette, four-column cards, value formatting, redraw/message system |
| Storage | Factory presets, user presets, projects, settings, sample slots and their on-flash formats |
| Samples | Built-in sample generation/data, zone selection, ADPCM decoding, upload validation |
| Connectivity | USB/TRS MIDI parsing and routing, MIDI output, USB audio |
| Development | Host simulator/tests, golden UI machinery, firmware packaging and size checks |

The only existing files that should need behavioral edits in the first implementation
are the boot/default path, physical-key routing, and home UI/input mapping. A small new
`ambient.c` module is preferable if it keeps scene constants and latch/macro policy out
of the generic engine code; it can still be included by `felucca.c` like the other
modules.

## Implementation sequence

### Step 1: freeze the baseline

- Run the current host tests and firmware build and record the result.
- Add an `AMBIENT_POC` build-time switch so the standard Felucca boot path remains
  available during development and regressions are easy to compare.
- Add tests for the desired ambient boot state before changing behavior.

Exit criterion: standard mode remains byte/behavior compatible at the source level,
and ambient mode has a testable initialization boundary.

### Step 2: make one excellent drone

- Apply the single ANALOG scene after Felucca defaults.
- Tune only existing parameters: oscillators, cutoff, envelope, level, delay send,
  reverb send, and shared effect settings.
- Mute unused tracks and bypass sequencer interaction in the ambient performance path.

Exit criterion: the firmware boots directly to the scene and a held chord sounds
stable without starting transport.

### Step 3: add correct LATCH

- Implement physical-key toggle latch above `trk_note_on/off`.
- Reuse panic/all-notes-off for clear and failure recovery.
- Test duplicate notes, chord mapping, the note cap, engine/scene reset, and rapid
  add/remove gestures.

Exit criterion: a player can build and edit a drone chord one note at a time with no
stuck notes and no arpeggiator retrigger.

### Step 4: expose SHAPE

- Give KNOB 1 a curated cutoff range and `SHAPE` label.
- Use ANALOG cutoff values 32..108 (about 146 Hz..6.25 kHz), presented as
  an intent amount from 0..100%; boot at cutoff 52 (26%).
- Show its value as a simple intent amount rather than filter frequency if that reads
  better in play testing.
- Keep the mapping in the ambient layer so the ANALOG engine remains generic.

Exit criterion: the full knob travel produces a useful dark-to-bright performance
gesture without silence, harshness, or abrupt jumps.

### Step 4b: focus the performance home

- Name the first curated scene `TIDE` and keep it visible with the current latch
  count (`0 / 4` through `4 / 4`).
- Present the intended four-macro vocabulary as `SHAPE`, `MOTION`, `DENSITY`, and
  `SPACE`; SHAPE and SPACE are active and the other two are explicit placeholders.
- Keep `FX + OCT-` visible as the clear gesture, and keep the existing page-family
  controls available as a development escape to the full Felucca UI.
- Preserve the latched-key light overlay so the screen count and physical keys tell
  the same story.

Exit criterion: the normal performance flow shows one scene, two working macros, the
four-note latch state, and no accidental edits through reserved macro knobs.

### Step 4c: expose SPACE

- Keep KNOB 4 as one immediate performance macro; use the existing FX pages for
  detailed delay and reverb editing rather than adding more HOME controls.
- Boot at 50%, anchored to the original TIDE send/mix values so there is useful
  travel in both directions.
- Use a staged curve: reverb establishes the room first, delay blooms above 35%,
  and the upper 40% gently trims only the dry path while keeping articulation.
- Fix delay time at 1/8 so live turns cannot create pitch jumps; increase feedback
  with SPACE but cap it at a stable value.
- Verify the gesture in the real DSP renderer: the 100% early and late tails must
  both carry at least 2.5 times the RMS energy of 0%, with bounded output.
- When the latch is cleared, balance every USB-MIDI Note On with an explicit Note Off.

Exit criterion: SPACE makes a plainly audible but controlled close-to-wash gesture in
rendered audio and on hardware, and clearing a latch also silences a software instrument
driven by the FM-1's USB-MIDI output.

### Step 4d: expose MOTION

- Keep KNOB 2 as one immediate slow-to-alive performance macro.
- Reuse the track LFO rather than adding a modulation process: a sine moves the ANALOG
  filter at about 0.07..0.24 Hz, with depth rising from still to clearly animated.
- Boot at 25%, where the movement is intentionally subtle.
- Keep pitch, amplitude, and shape modulation at zero for this first hardware pass;
  pitch modulation's existing parameter resolution is too coarse for gentle drift.
- Leave the detailed LFO pages available outside the focused HOME screen.

Exit criterion: a held chord can be completely still at 0%, breathes slowly at the
default, and is plainly animated but stable and in tune at 100%.

### Step 4e: expose DENSITY

- KNOB 3 morphs oscillator mix 0..64 in the lower half, retaining the equal
  blend above 50%; detune follows 0 / 16 / 36 cents and chorus send 0 / 52 / 100.
- Boot at 50% for exactly the accepted TIDE parameters. Retain the existing
  chorus rate/depth, polyphony, envelopes and all other macro settings.
- Use existing DSP and parameter storage; the macro amount resets with the scene.
- Verify endpoints, unrelated parameters, four-note latch and rendered audio
  stability before hardware listening. The upper tuning awaits listening approval.

### Step 5: hardware feel test

- Build and package with the existing toolchain.
- Only after explicit approval, flash one FM-1 using the existing recovery-safe path.
- Evaluate boot time, key/latch reliability, knob range, output level, noise, CPU
  headroom, and whether the box invites several minutes of uninterrupted play.

This is the v0.1 decision point. Only if it feels convincing should the project add
GRAIN-based TEXTURE/FREEZE and additional curated scenes after balancing the four
TIDE macros and integrating upstream Felucca 1.5.1 on a dedicated branch. REV THROW
belongs to performance FX rather than the persistent SPACE macro.

## Acceptance criteria for the proof of concept

- Power-on lands on one named ambient scene with no setup menu.
- Playing and latching up to four notes produces a continuous drone without transport.
- Pressing a latched note again removes only that note.
- One clear gesture reliably stops every latched note.
- KNOB 1 performs an audibly useful, smooth dark-to-bright SHAPE sweep.
- KNOB 2 moves from still through subtle filter breathing to clearly audible motion
  without pitch instability.
- KNOB 4 performs a bounded close-to-wash SPACE sweep whose rendered early and late
  tails are materially stronger at 100% than 0%.
- Tracks 2-4, drums, and sequencer activity do not appear in the normal performance
  flow.
- Existing audio safety, recovery, storage formats, and non-ambient build behavior are
  preserved.
- Host tests pass and the firmware fits and builds for FM-1 before any hardware flash.
