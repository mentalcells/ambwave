# PULSE third-layer prototype results

For the current follow-up after hardware feedback, see
`PULSE_PRESET_FIX_RESULTS.md`. This document records the initial prototype.

Implemented 2026-10-11 on `codex/ambient-arp`, based on accepted TEXTURE and
handoff `e05cc37`, fast-forwarded into `ambient-prototype`. Only one checkout
was registered and the starting tree was clean. The unchanged upstream build
and host baseline remains the recorded milestone 0 in AMBIENT_PLAN.md.
The implementation commit is `2bb2514`. PULSE tuning awaits hardware listening.

## Playing and mappings

Boot remains silent, selected TIDE. ALGORITHM on curated HOME reaches layers
1–3; EDIT retains all four tracks. Layer 4 stays muted. Hold HOME for MIX:
knobs 1/2/3 balance TIDE/TEXTURE/PULSE; knob 4 is inactive. FX + OCT- clears
only the selected layer, including its incoming MIDI owners and outgoing held
input notes. Selection leaves the other layers sounding.

PULSE uses PHASE BELL with attack 4, decay 76, zero sustain, release 42 and
level 72/127. Its physical keyboard toggle-holds up to four pitches, with no
raw input chord sounding. Key release retains them; another tap removes the
pitch. PLAY generates ascending, sorted notes over one octave; STOP releases
the generated gate but keeps input pitches. Restart begins from the first
pitch immediately. Existing preset steps are cleared at boot, as before;
full Felucca editing, sequencing, recording and arp pages remain available.

| Macro | Mapping |
| --- | --- |
| SHAPE | PHASE distortion depth 20–85, default intent 35% |
| MOTION | Existing smooth irregular DRIFT LFO, shape depth 0–18, rate 6–30; no pitch modulation |
| DENSITY | 0–24%: 2 bars; 25–49%: whole note; 50–74%: half note; 75–99%: quarter note; 100%: eighth note |
| SPACE | Layer-local delay 0/30/90 and reverb 0/70/110 at 0/50/100%; dry attenuation above 60% |

The gate is 24/128 of each interval; probability is fixed at 127 (every
scheduled event). Felucca does implement probability, but this prototype does
not need random skips. TIDE alone retains ownership of the shared FX settings.
PULSE default DENSITY is 25%, one event per bar. SELECT already controls global
BPM, shown in the header; existing BPM LOCK and GLO + SELECT override remain.
No additional tempo gesture is introduced.

## Audit and implementation boundaries

Felucca's internal arp advances without transport; its external-clock mode
uses clock-derived samples, transport commands and cable-loss stop. PULSE adds
a curated-track transport guard and immediate restart, keeping other arp
tracks' behavior. External MIDI clock routing, tempo estimation, timeout and
sequencer transport are unchanged. No allocator, DSP, project/package format,
bootloader, loader or recovery changes were made.

Felucca HOLD replaces the old input chord when the next physical chord starts.
PULSE instead keeps a separate four-pitch physical ledger. The existing arp
`held[]` is the union of physical and incoming MIDI pitches. Releasing either
owner preserves the other's input. Panic/load/project/all-notes-off paths
forget physical inputs and balance USB-MIDI Note Ons. Outgoing MIDI continues
to represent held controller input, as in Felucca; generated arp notes are
local audio, not a new MIDI-output sequencer. A scoped generated-gate flag
handles pitch zero, which upstream otherwise uses as its arp idle sentinel.

Candidate WAVs render existing PHASE BELL, ANALOG PLUCK and PHYS PLUCK with a
common short release in `build/pulse_demo/candidate-*.wav`. PHASE is chosen for
its distinct digital pluck and small existing render loop (target cost 77 vs
ANALOG 581 in this build); PHYS retains additional model state and cost. These
are software comparisons, not user approval or a claim of hardware audition.
The final softened PULSE and combined scene examples are separate WAVs.

## Software evidence

`tools/build-hasake-macos.sh` and `tools/test-macos.sh` completed successfully
(`ALL HOST TESTS PASSED`), including host/target CPU, 92 unchanged golden
renders, storage/USB/MIDI/recovery, UI and emulator tests. Emscripten used the
local `.toolchains/emscripten-cache`. Optional DaisySP float-reference checks
were skipped because DaisySP is absent; PHYS stability checks passed.

- Required Hasake build passes: app 436660 bytes; RAM data+BSS 89788/98304;
  pool 330208/344064. Target render-cost checks retain existing thresholds:
  audio interrupt cost 39083 against baseline 37502, within its +10% allowance.
  The transport guard sits outside the arp note-list loops; bounded physical
  and MIDI ownership helpers remain separate from inlined block loops. No
  target-budget entries or golden audio hashes were changed.
- New PULSE checks cover silent stopped input, toggle/removal/cap, generated
  gates, STOP/restart/clear, pitch zero, MIDI ownership/balanced output,
  panic/load, MIX/selection, FX isolation and clock-driven advancement.
- At 120 BPM, sixteen seconds produces exactly 4/8/16/32/64 events across the
  five DENSITY divisions. Pausing external-clock advancement adds no events;
  advancing one division generates one new note.
- One hundred repeated dense arp/input changes with TIDE chord replacements
  and TEXTURE register changes preserve all five sustained gates within the
  shared eight-voice budget at curated release settings.
- All sixteen PULSE macro corners at maximum three-layer levels: peak 18907,
  worst absolute DC 17.6, worst late RMS 43.7 eleven seconds into a twelve-second
  render, after STOP/clear at six seconds. No clipped or stalled tails.
- Isolated TIDE and TEXTURE WAVs are byte-identical to an unchanged `e05cc37`
  source snapshot using the same harness. SHA-256 respectively:
  `38134e1eb487751851197d55b3f2dbabccf809262da33a2c468e42e8cd0b9248` and
  `21f0cae0ab2a100df6aba56a17f0f53004620298daac3d257d702293dcd02bff`.
- Default solo PULSE RMS is 130.1, dense PULSE 334.2 and the three-layer demo
  7596.0 over seconds 2–6. Default solo peak is 2280; combined peak 17566.
  This intentionally restrained balance is provisional for listening.
- HOME/MIX lint spans three selected layers, every theme, FLAT/LINE and
  normal/large cards: zero clipping, overlap, ink, neutrality or alignment
  findings. Normal/large PULSE and MIX renders were visually inspected.
- AMBIENT_POC=1 emulator checks include third-layer input, PLAY/STOP and native
  parity over all three layers, register changes, STOP/restart and clears:
  220500 frames match bit for bit. The ordinary heavy-song comparison also
  matches 88200 frames. Host timing is not a measured FM-1 cycle count.

## Limits and hardware listening checklist

Eight shared voices still permit external MIDI overload to steal held voices.
The tested headroom applies to the curated short pluck release; making release
very long or adding full-UI sequence notes can consume it. The input count is
an ownership ledger, not a guarantee that an overloaded held voice still sounds.
External MIDI can add more than four arp pitches; four is the panel-input cap.
FX are shared buses with independent sends, not three separate reverbs. Macro
intent, layer identity and MIX are runtime-only; scene persistence is deferred.

After explicit approval in the installing conversation:

- Hold four TIDE notes and TEXTURE, add PULSE inputs while stopped, then PLAY.
- Sweep SHAPE/MOTION and each DENSITY division; assess softness and sparse pacing.
- STOP/restart PULSE while both sustained layers continue; toggle inputs stopped.
- Hold HOME to balance three layers, including fades to zero and back.
- Clear each layer independently; test octave changes and repeated chords.
- Compare local PULSE SPACE sweeps with TIDE SPACE at 0/50/100.
- Assess clicks, dropped voices, long tails and actual device CPU margin.
- Enter EDIT; check tempo/BPM LOCK, full track access and external clock stop.

The user subsequently approved installation in this chat; the PULSE sound has
not yet received hardware listening feedback.
Scene saving and upstream integration remain separate milestones.

## Package

`build/hasake-pulse.fwsc`, 609849 bytes, SHA-256:
`0b5ce55a8dae3883f39025d1b97cf51407dae4c960b5167aee1a83e8a8e56e0c`.

## Hardware installation

On 2026-10-11 the user explicitly requested “can you please flash to fm1”.
Installed the package above with `tools/fm1_install.py --port Felucca --yes`.
The installer completed the write and restart successfully; a separate `--info`
check confirmed `FM-1_900 [running]` on the Felucca MIDI port. This records
installation of implementation `2bb2514`, not acceptance of sound tuning or
completion of the listening checklist. No preset/project erase was requested.
