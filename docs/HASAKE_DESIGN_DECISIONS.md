# Hasake design decisions and next steps

Updated 2026-10-14. This is the durable design record for implementation chats.
Read alongside AMBIENT_PLAN.md. Separate accepted decisions from proposals below.

## Accepted product direction

Hasake is an immediate layered ambient instrument inspired by Sonicware LIVEN
Ambient Ø's play/hold, transform, add a layer and balance workflow. A single
macro preset was a useful first proof, but did not establish the whole instrument.

- A **sound** is a curated engine and its SHAPE/MOTION/DENSITY/SPACE mappings.
- A **layer** is a track playing that sound through held notes, an arp or a sequence.
- A **scene** is the combined soundscape: layers and their settings/playing behavior.
  A scene is not another name for a track. Scene selection and storage are not implemented.
- The four macro concepts remain consistent; their mappings can differ by sound.
- No menu diving means immediate everyday performance, including layer selection,
  sound transformation and mixing. Detailed editing can remain accessible.
- Reuse Felucca engines, sequencer, arp, modulation, FX, MIDI, storage and recovery.
  Expose features where they serve the ambient playing flow; do not remove them
  merely because the current HOME does not use them.
- Note hold is one playing behavior. Future clocked textures can use arps or
  sequences. There is no accepted requirement that every sound be a drone.

Reference discussed: https://sonicware.jp/pages/liven-ambient
This is workflow inspiration, not a commitment to matching its synthesis or capacity.

## Implemented and heard on hardware

TIDE is an ANALOG sound on layer 1 with four-note toggle hold and all four macros.
Accepted MOTION uses smooth irregular filter DRIFT; pitch remains stable.
DENSITY preserves the accepted TIDE sound at 50%, with a thinner lower endpoint
and detuning/chorus width above the midpoint. Accepted commit f383c1b.

The two-layer prototype is commit f893254 on codex/ambient-texture, with spec
22b4054. TEXTURE uses NOISE ANLG on layer 2, one held voice, colored/filtered
noise and optional impulses. ALGORITHM selects layers; hold HOME for MIX;
FX + OCT- clears the selected layer; HOME shows sound identities prominently.
Layer selection keeps the other layer sounding. PLAY retains transport semantics.
See TEXTURE_PROTOTYPE_SPEC.md and TEXTURE_PROTOTYPE_RESULTS.md for exact behavior.

The user manually installed TEXTURE in the implementation chat and reported:
“amazing, it adds a lot. I tried it with tide. controling the mix is good.”
In this chat the user confirmed: “The texture+ tide works well.”
This accepts the basic two-layer sound/performance direction. It does not claim
that every extreme, external MIDI overload or hardware CPU condition was tested.

## Limits we have accepted for the prototype

- Eight voices are shared. Physical four-note TIDE plus one TEXTURE hold passed
  software stress tests; external MIDI overload can still steal held voices.
- FX buses are shared. TIDE owns global delay/reverb settings; TEXTURE SPACE
  changes its own sends/dry gain. Fully independent reverbs are not implemented.
- Macro intent/curated ownership is runtime-only. Existing project loading can
  leave curated mode; saving engine parameters alone does not constitute scene recall.
- Scenes must start silent on recall if that proposed behavior is adopted;
  held physical notes are performance state, not automatically restored notes.
  This recall rule is proposed, not yet implemented or finally specified.

## Foundation choice

Keep Felucca for the next milestone: existing engines and infrastructure plus
successful hardware layering support continuing. This is an evidence-based
working choice, not a permanent commitment. Reassess if measured voice pressure,
shared-FX behavior, CPU/RAM or required texture quality blocks the instrument.
Changing firmware alone would not eliminate the FM-1 hardware limits.

## Original roadmap and deliberate change

Original follow-on order: balance/stabilize TIDE's four macros, integrate the
planned upstream Felucca update on a dedicated branch, then add contrasting sounds.
The update was referred to as “1.5.1” in the earlier handoff; verify the exact
upstream release/tag before integration rather than guessing a ref.
REV THROW belongs to performance FX and must not be folded into persistent SPACE.

Deliberate change: we tested a second simultaneous TEXTURE layer before that
integration, to resolve the basic interaction and test Felucca's fitness. It worked.
Do not discard this evidence or revert to a library of mutually exclusive presets.

## Revised next order (user accepted)

After the proposed save-first order, the user asked whether another layer was
more useful. We agreed: scene saving is implementation-heavy, but that does not
make it the highest product priority. The next milestone is a sparse plucked
arp layer alongside TIDE and TEXTURE: harmony, texture and discrete events.
The user requested a new implementation chat for this milestone.

1. Audit existing arp behavior and shared voice headroom, then implement layer 3
   with direct note holding, PLAY/STOP and independent MIX. Preserve accepted
   TIDE/TEXTURE tuning and reuse Felucca's existing arp and DSP.
2. Evaluate three-layer interaction on hardware before expanding sounds.
3. Upstream integration remains a separate technical milestone: verify the exact
   release, then choose its position before more feature work or persistence.
   Do not combine integration with this arp prototype.
4. Specify scene save/recall once the playing model is clearer; preserve existing
   projects and define macro/identity/mix/shared-FX restoration before coding.
5. Curate a small combined-scene set; granular/sample expansion remains future work.

See ARP_LAYER_PROTOTYPE_SPEC.md for the next implementation handoff. Hardware
installation always requires explicit approval in the chat doing the install.

## Third layer implementation (software evaluated)

PULSE is implemented on `codex/ambient-arp` from `e05cc37`: a softened PHASE
pluck driven by Felucca's existing arp. Physical input toggles up to four
pitches; PLAY/STOP controls generation and retains input. Three-layer MIX uses
knobs 1–3. SELECT retains direct tempo and BPM-lock behavior. DENSITY uses
2-bar/whole/half/quarter/eighth divisions; SPACE affects only local sends/dry.
See ARP_LAYER_PROTOTYPE_RESULTS.md for the audit, exact mappings, software
validation, reference hashes, package and hardware checklist.

The accepted TIDE/TEXTURE isolated reference renders remain byte-identical.
Curated three-layer voice-pressure and macro-corner checks pass. Shared buses,
external MIDI stealing and runtime-only identity/macros remain limits. Software
results support hardware evaluation; PULSE sound, balance and device CPU margin
are not hardware-approved. The user explicitly requested flashing in this chat
on 2026-10-11: implementation `2bb2514` was installed successfully and restart
was verified (`FM-1_900`, running). The first feedback rejected the sound/pacing and found the HOME preset-loss bug;
see the follow-up below.
The next product step remains three-layer listening, then separately scheduled
upstream integration and scene persistence as previously decided.

## PULSE hardware feedback and role clarification

After installing `2bb2514`, the user found the slow ordered notes and preset
unconvincing, and lost PULSE's curated controls when changing PRESETS. The bug
was the general HOME preset loader clearing curated identity/performance state.
This sound tuning has not been accepted.

The user explicitly chose PULSE's role: “Scattered plucks: randomly choose from
my held notes”. Treat it as discrete pitched details over TIDE harmony and
TEXTURE noise, not another sustained warm layer. Random choices stay in the
held set; DENSITY controls clocked spacing. The revision keeps the layer and
inputs while HOME PRESETS cycles warm pluck, soft mallet and bell pluck, and
starts at half-note pacing. All use existing PHASE/arp infrastructure. See
PULSE_PRESET_FIX_RESULTS.md for behavior, validation, package and limits.
No persistence or upstream integration was added. Sound tuning still requires
user listening feedback. The revision was installed on 2026-10-12 using the
explicit flash approval already given in this conversation, with successful
write/restart and an independent running identity check.

## PULSE macro, speed and mute decisions (2026-10-12)

The user accepted R2 as a beginning, but found its knobs too subtle. They
approved more obvious SHAPE (damped to ringing), MOTION (per-hit variation),
DENSITY (hit probability), and SPACE (close to distant) mappings. Speed is a
separate musical control: HOME + knob 4 selects half through sixteenth notes;
SELECT remains master tempo. DENSITY no longer owns the speed division.

HOME + the first three black keys toggles TIDE / TEXTURE / PULSE audio mutes.
Mutes retain notes, level and clock position; echoes finish naturally. They
use runtime performance gain rather than the legacy P_MUTE flag, which only
blocks new notes. The user authorized proceeding with this design. See
PULSE_CONTROLS_RESULTS.md for the implementation, evidence and package. The
new sound remains under hardware listening evaluation. The tested R3 package
was installed with the conversation's existing explicit flash approval;
write/restart and an independent running identity query succeeded. No scene persistence
or upstream integration was added.

## Granular prototype (2026-10-13)

The user authorized starting a granular implementation on a branch. The prototype
on `codex/ambient-granular` uses existing GRAIN with built-in FLUTE material as
CLOUD, selectable instead of TEXTURE on layer 2 through HOME PRESETS. It retains
TIDE and PULSE and the three-layer mixer. Sound changes clear the layer-2 hold;
macro defaults reset while mix and shared global FX are preserved. No fourth
layer, upstream merge, new DSP, scene persistence or hardware installation is
included. Mapping and sound quality remain provisional until listening. See
GRANULAR_PROTOTYPE.md for controls, software checks and limits.

## Independent sound slots (2026-10-14)

The user clarified that CLOUD was intended as another layer, and proposed
selecting any of TIDE/TEXTURE/CLOUD/PULSE on every track. They authorized this
change and requested the emulator after implementation. The sound/slot split
on `codex/ambient-sound-slots` supersedes fixed-slot selection: four default
layers are TIDE, TEXTURE, PULSE and CLOUD; all share the same HOME sound list.
Each layer owns notes, macros, level, mute and PULSE clock state. Sound changes
start silent for that layer, preserve HOME/mix/mute/sequence and leave other
layers intact. MIX uses four level knobs; selected PULSE speed moves to
HOME + SELECT. SPACE uses local sends/dry on every sound, with shared global
FX initialized to the accepted TIDE midpoint. No persistence or upstream merge
is added. See SOUND_SLOTS_RESULTS.md for evidence and remaining hardware checks.

## Independent playing-mode experiment (2026-10-14)

After testing sound slots, the user wanted to sequence CLOUD notes and asked
whether layers were tracks or fixed roles. They authorized an experiment on a
separate feature branch. `codex/ambient-playing-modes` keeps four tracks and
separates HOLD/SCATTER/SEQUENCE from the selected sound. HOME + ARP cycles mode;
SEQ from HOME selects SEQUENCE and opens the existing STEP editor. PRESETS
retains the mode and pattern, clearing live input as before. HOME + SELECT
controls selected SCATTER/SEQUENCE speed. The original startup sound/mode
combinations remain. No new DSP, persistence format or hardware installation is
included. This remains a workflow and tuning experiment pending user feedback;
see PLAYING_MODES_RESULTS.md for behavior, evidence and limits.

## Playing-mode hardware installation (2026-10-14)

The user explicitly asked to flash the playing-mode experiment in this chat.
Implementation `4c42c71` was rebuilt and installed successfully; package
validation, 100% write and restart completed. A separate identity query confirmed
`FM-1_900 [running]` on `Felucca`. See PLAYING_MODES_RESULTS.md for the package
hash. Hardware workflow/sound/CPU evaluation remains pending user feedback.

## Updating this record

After a hardware or design decision, record the commit, user feedback, accepted
behavior, remaining limits and any roadmap change here. Update AMBIENT_PLAN.md's
status/link. Keep specifications as milestone contracts and results as evidence;
this file holds decisions across chats. Flashing always needs explicit approval
in the conversation doing the install.
