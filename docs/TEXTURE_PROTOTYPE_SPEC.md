# Hasake two-layer TEXTURE prototype

Status: implementation specification approved in principle on 2026-10-11.
Inspiration: Sonicware LIVEN Ambient Ø's immediate layering workflow.

## Goal and vocabulary

Within 30 seconds, latch a four-note TIDE chord, add a separately evolving noise
texture, and balance both without entering an edit page. This is a test of
Felucca as a layered ambient foundation, before a larger sound library.
A sound is a curated engine plus macro policy; a layer is a track playing that
sound; a scene is the whole combination. Scene selection/storage is future work.

## Baseline and scope

Accepted TIDE: commit f383c1b on codex/ambient-density, based on accepted MOTION
db77e1d. DENSITY was built, passed the full host suite and Hasake emulator,
flashed successfully, and received positive four-note hardware listening feedback.
The working tree was clean before this specification was added.
ambient-prototype already contains MOTION but does not yet contain DENSITY.
Merge accepted DENSITY into ambient-prototype, including this spec, then create
codex/ambient-texture. Check repository state before doing so.

Implement two active layers only: TIDE on track 1, TEXTURE on track 2. Tracks
3/4 stay muted and sequencers empty. No new DSP, file formats, sample imports,
scene persistence, arp or sequence UI in this milestone. Leave underlying
sequencing, USB, MIDI, saving and recovery working through the full UI.
Upstream integration is separate work; do not mix it into this prototype.

## Performance controls

- Boot selects TIDE, preserving its accepted sound and four macro defaults.
  Both layers start silent with empty held-note state.
- ALGORITHM selects either active layer on ambient HOME. Selection changes
  keyboard/macro focus only; sounding notes, tails and macro values persist.
  Scope the two-layer restriction to ambient HOME; retain full UI track access.
- Four knobs edit SHAPE, MOTION, DENSITY and SPACE for the selected layer.
- Hold HOME to show MIX: knob 1 controls TIDE level, knob 2 TEXTURE level;
  knobs 3/4 display inactive. Release restores macros. Implement only after
  auditing button conflicts, press/release ordering and HOME navigation;
  suppress macro turns while MIX is held. Preserve standalone HOME behavior.
- TIDE keyboard retains four-note toggle latch.
- TEXTURE keyboard holds one noise voice. Press from silence starts it; press
  the same mapped note again releases it; press another mapped note moves its
  register without accumulating voices. Octave and scale mappings remain valid.
  Avoid audible retrigger gaps when changing register where existing paths permit.
- FX + OCT- clears the selected layer, balances its outgoing MIDI note events,
  and leaves the other layer intact. Layer change is never a clear operation.
- PLAY retains transport semantics; it does not kill held layers. No automatic
  pattern creation. Preserve a route to the existing full editing interface.
- Independent MIDI ownership: external note-off must not release a physically
  held texture or TIDE note. Preserve existing routing policy. Assess external
  MIDI polyphony against the shared budget without broad protocol changes.

## HOME visibility

Prominently display selected layer and sound: '1 TIDE' or '2 TEXTURE', rather
than relying on the current subdued bottom-right TIDE label. Show TIDE held
count /4 and TEXTURE ON/OFF plus an overview of both layer levels/activity.
MIX visibly labels both sounds and levels. Ensure all supported themes and
normal/large card layouts fit. Keep battery/USB and critical messages legible.

## TEXTURE sound and macro policy

Use existing NOISE engine in ANLG mode: a soft filtered, colored noise bed with
optional random impulses. Keep mode fixed; no LFSR/META or bit crushing. One
held voice, gentle attack/release, conservative resonance and initial level.
Initial macro targets: SHAPE 35%, MOTION 25%, DENSITY 25%, SPACE 50%.
Exact engine values must be selected from measured renders, not treated as
already approved sound tuning. Retain original TIDE parameters as the reference.

- SHAPE: dark/soft through airy; map noise color and filter frequency using
  bounded musical curves. Key register has restrained filter tracking.
- MOTION: 0 is still; raise smooth irregular filter movement using the existing
  LFO DRIFT. Avoid stacking competing intrinsic NOISE drift and track LFOs.
- DENSITY: quiet continuous bed remains at 0; add sparse crackle/impulses toward
  the top using NOISE's existing DENS parameter, with bounded compensation only
  if measured loudness requires it. Mixer controls overall level separately.
- SPACE: close/dry through diffuse using per-track delay/reverb sends and, if
  necessary, the existing per-track dry-gain mechanism generalized to layer 2.

Felucca effects are shared buses. For this prototype retain TIDE's accepted
SPACE mapping as the owner of global bus settings. TEXTURE SPACE changes only
its own sends/dry gain: it must not rewrite global delay/reverb parameters or
TIDE's sends. Its perceived space consequently depends on shared TIDE bus
settings; disclose and evaluate this limitation. Selection must never reapply
bus settings or reset effects. Fully independent reverbs are out of scope.

## Voice and stability requirements

Felucca has eight voices shared across four parts. Four held TIDE notes plus
one held TEXTURE voice must coexist. Account for release tails and repeated
chord/register changes; do not claim five active notes guarantee headroom.
Use existing allocation/steal behavior where possible. If reliable coexistence
needs allocation changes, demonstrate the failure and make the smallest
ambient-scoped policy; do not silently alter global voice limits or unison.
Fades, mute/level changes, clear, layer switching and all extreme macro
combinations must avoid hangs, sharp discontinuities and sustained clipping.

## Validation and delivery

Read AGENTS.md and AMBIENT_PLAN.md before changes. Baseline unmodified upstream
build/tests were already established; preserve the official upstream remote.
Build with tools/build-hasake-macos.sh (calls ./build.sh with AMBIENT_POC=1),
then run tools/test-macos.sh (calls sh tests/run_tests.sh). Run Hasake mode
emulator checks with AMBIENT_POC=1. Docker and Emscripten cache access may need
sandbox escalation; prior full suite failure was cache permission only and
passed after authorized escalation. Do not alter golden hashes casually.

Add meaningful checks for independent layer focus/hold/clear and MIDI ownership;
MIX input conflicts; resetting/loading/engine changes; macro isolation including
shared FX settings; four-note TIDE plus texture under repeated changes; CPU
budget, rendered clipping/DC/tails, storage compatibility, HOME layout and
native/WebAssembly parity. Provide listenable comparison renders if practical.
Compare TIDE alone at default settings with the accepted baseline.

Deliver a clean topic-branch commit, test/build results, firmware package and
short hardware listening checklist. Hardware test: latch TIDE chord, switch to
TEXTURE and start it, change its register/macros, fade both, clear one at a time,
repeat chord changes, then evaluate whether layering feels immediate.
NEVER flash without explicit user approval in the new conversation. Earlier
approval covered DENSITY only and does not authorize TEXTURE installation.

## Foundation decision

Pass: useful distinct texture, stable combined sound, direct layer selection and
mixing, acceptable measured CPU and voice behavior. If shared FX, voice pressure
or resource limits block the desired experience, report concrete evidence and
reassess Felucca before expanding scenes. Do not rewrite firmware prematurely.
