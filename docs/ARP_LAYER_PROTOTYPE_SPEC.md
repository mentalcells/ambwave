# Hasake third-layer arp prototype

User accepted the direction and requested implementation handoff on 2026-10-11.
Read AGENTS.md, AMBIENT_PLAN.md and HASAKE_DESIGN_DECISIONS.md first.

## Starting point

Clean codex/ambient-texture at f893254 with design record 31c4c20 (plus this
handoff documentation commit). TIDE + TEXTURE was manually installed and heard
positively by the user; MIX is useful. ambient-prototype currently stops at
22b4054. Verify branch state, merge accepted TEXTURE and these docs into
ambient-prototype, then create codex/ambient-arp. Do not overwrite concurrent work.

## Product goal

Build harmony (TIDE), continuous texture (TEXTURE) and sparse discrete events
(layer 3) together without editing pages. Saving is deferred; no scene-library,
new DSP, new persistence format or upstream integration in this milestone.
Working layer name: PULSE; this is a provisional label, not a fixed sound design.

## Performance behavior

- Boot with all layers silent, selected TIDE, existing TIDE/TEXTURE tuning intact.
- ALGORITHM selects layers 1..3 on ambient HOME; preserve full UI track access.
  Track 4 remains muted. Selection never releases other layers or resets macros.
- Layer 3 uses a curated soft pluck from an existing suitable engine. Audition
  candidates and choose a low-cost source that contrasts with sustained ANALOG
  and noise. Do not default to another sustained pad or claim tuning is approved.
- Keyboard toggle-holds a small set of pitches feeding the existing arpeggiator;
  release of a physical key retains that input set. Same pitch toggles off.
  Use at most four input pitches, but do not sound them as a simultaneous chord.
- PLAY starts/stops the arp's clocked note generation; TIDE and TEXTURE continue.
  STOP releases arp-generated notes, keeps input pitches available for restart,
  and never clears the two sustained holds. Do not accidentally run preset steps.
  Preserve existing transport behavior in full UI and incoming MIDI clock policy.
- Hold HOME for MIX: knobs 1/2/3 are TIDE/TEXTURE/PULSE levels; knob 4 inactive.
- FX + OCT- clears selected layer and balances MIDI. PULSE clear empties input
  pitches and releases its generated note(s), leaving other layers untouched.
- Display layer identity, held-input count and running/stopped state prominently;
  show all three levels/activity, retaining UI message/USB/battery priority.
- Define a direct tempo control using existing hardware (audit the current tempo
  path before proposing a new gesture). Expose tempo clearly when relevant.
- Keep EDIT and full Felucca pages available; ordinary tracks retain behavior.

## Four macros

Keep SHAPE/MOTION/DENSITY/SPACE on the selected layer. Choose a concrete mapping
based on existing engine/arp capability after auditing it, and explain it before
implementation; routine parameter choices do not require another approval.
SHAPE should transform pluck tone. MOTION should add evolving timbral variation
or existing pattern movement, distinct from DENSITY. DENSITY must make events
sparser/busier using existing arp timing/gate controls where possible; verify
actual event count and clock synchronization, not just parameter values. Do not
promise probability/skips if the existing arp does not implement them. At low
DENSITY the layer remains useful; avoid a permanently busy groovebox feel.
SPACE changes layer-local sends/dry gain. TIDE remains global FX-bus owner, as
in TEXTURE; selecting PULSE must not reapply global FX settings.

## Audit before changes

Trace Felucca arp input ownership, held-note handling, scheduler/transport,
generated note-offs, external MIDI and clock, and layer clear/load/panic paths.
Choose the smallest ambient-scoped integration, retaining upstream arp behavior
outside curated HOME. Determine whether changing hold inputs while stopped
silently queues them rather than sounding raw notes. Identify any conflict with
existing physical latch policies and keep outgoing MIDI balanced.

Eight voices are shared: four TIDE holds + one TEXTURE + pluck release tails.
Test repeated input/chord/register changes and long tails; do not assume three
tracks fit just because six notes usually sound. Start conservative pluck release
and polyphony; avoid global allocator changes. External MIDI can steal physical
held voices in the accepted prototype; do not quietly claim this is fixed.

## Validation and completion

Build with tools/build-hasake-macos.sh, then tools/test-macos.sh; required host,
target CPU budget, regression renders, storage/USB/MIDI/recovery and UI checks
must pass. Run AMBIENT_POC=1 emulator and native/WebAssembly parity. Cache access
may require escalation. Do not change golden hashes without reviewed intent.
Add meaningful tests for arp input updates, note-offs at STOP/clear/load/panic,
restart, timing/density, MIDI ownership, MIX/macro isolation, three-layer voice
pressure and macro corners; render audible examples and verify clipping/DC/tails.
Compare isolated accepted TIDE and TEXTURE references to detect regressions.
Check HOME/MIX across themes and normal/large layouts, including tempo/state.

Deliver a clean topic commit, package, software evidence and hardware listening
checklist. Update results/design record with evidence and limitations. Do not
merge sound tuning as hardware-approved before user listening.
NEVER FLASH without explicit approval in this new implementation conversation.
Prior installation approvals do not authorize this package. Report concrete
Felucca limitations if they block the desired flow rather than starting a rewrite.
