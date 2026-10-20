# Separate track roles and sound patches

2026-10-20. Branch `codex/role-sound-separation`, based on `ambient-prototype`
(`828ceaa`). Existing uncommitted Tide listening experiments remain present;
this change does not enable them or adopt a new factory sound.

## Playing model and controls

Four independent tracks each have a role, a compatible patch, and an independent
HOLD / SCATTER / SEQUENCE playing mode. Roles retain the existing musical
policies; this adds no generators or DSP engines.

- ALGORITHM selects the track.
- HOME + PRESETS cycles TIDE / TEXTURE / CLOUD / PULSE.
- PRESETS cycles the patches belonging to the selected role.
- TIDE: BODY PAD (original Tide), WARM PAD (ANALOG), SILK PAD (FM6).
- TEXTURE: NOISE. CLOUD: GRAIN. PULSE: WARM PLUCK / SOFT MALLET / BELL PLUCK.
- HOME + ARP, HOME + SELECT, HOME mixing/mutes and the four macros retain their
  existing purpose. The role appears in track rows; the selected patch appears
  in the footer, alongside the Cloud length when applicable.

A role change selects its default patch (ID 0). A patch change cannot change the
role. Both retain playing mode, timing choices, written pattern, level, pan,
ambient mute and shared FX, while clearing local live notes and resetting macros
to that patch's defaults. Existing Cloud first-clock speed policy still applies
until a speed has been explicitly chosen. There is no per-role patch memory.
Turning PRESETS on a one-patch role, or making a full-circle selection, does not
reset macros or interrupt held notes.

## Implementation boundary

`ambient_layer_t` in `firmware/src/seq.c` now names its independent `role` and
`patch` fields explicitly. Their numeric values remain unchanged. Patch IDs are
stable within their role: the pair identifies a curated patch.

A small static catalog in `firmware/src/ambient.c` records allowed role/patch
pairs, engine and display name. Unsupported pairs are rejected before clearing
notes or modifying any state. Lookup in the audio path is constant-time. Existing
patch initialization, macro mappings, envelope policy and engine DSP are reused;
there is no generalized adapter system or assumption of cross-role compatibility.

`ui_input.c` handles the HOME + PRESETS gesture. `ui_draw.c` and `ui_graph.c`
separate the role labels from the selected patch. `ui.c` snapshots curated
configuration for sound undo/redo and finalizes the undo signature after the
complete patch setup. Live held notes are not restored by undo.

The browser mixer keeps the role as its compact label and exposes the patch in
its tooltip. Browser state-export bit layouts remain unchanged.

## Storage and scope

No project, user-preset, loader, bootloader, recovery or package format changes.
Existing project and arbitrary preset loads continue to leave curated HOME
ownership; this is not scene recall. Role, patch, macro intent and other ambient
metadata remain runtime-only. Complete scene persistence requires a separate
versioned storage design, as agreed in the review plan.

No hardware was flashed. On-device gesture comfort and listening acceptance
remain to be checked by the user.

## Validation

- `tools/test-macos.sh` / `sh tests/run_tests.sh`: ALL HOST TESTS PASSED.
  All 92 golden renders are unchanged; zero health, voice/routing or CPU-budget
  failures. Project/user-preset storage and target-loop budgets pass. The
  optional PHYS reference check is skipped because DaisySP is absent.
  The complete suite was rerun with compiler-cache access after its sandboxed
  browser build was blocked by Emscripten's installed-cache permissions.

- `tools/build-ambwave-macos.sh` (calls `./build.sh`): 441628-byte app;
  static RAM 89948/98304, pool 330208/344064. Docker socket access required
  running the existing build wrapper outside the filesystem sandbox.
- Focused host checks cover both physical selector gestures on all four tracks,
  compatible patch selection, MIDI note-off balance, retained neighboring tracks,
  mix/pattern/mode preservation, invalid-pair rejection, wraparound, silent
  single-patch browsing and curated undo/redo.
- An independent original-source snapshot at `828ceaa` was compiled alongside
  this source. Audio hashes match exactly for all eight patches, all three modes
  and sixteen macro corners (384 one-second renders; SEQUENCE patterns empty).
  The opt-in Tide experiment is disabled for this comparison.
- HOME/MIX layout sweep: 184320 screens, zero lint or alignment findings;
  the generated Cloud HOME screen was also inspected visually.
- AMBIENT_POC=1 WebAssembly checks pass role selection on every track, patch
  browsing, phrase retention, mixer, transport and persistence compatibility.
  Heavy-song audio, the layered performance and SILK/WARM onset/release renders
  match the native build bit for bit.

Local review preview: `http://127.0.0.1:8793/` serves `build/emu/roles/`.
In the browser, Shift-click HOME to latch it, turn PRESETS to change role,
and Shift-click HOME again to release before changing patches.
