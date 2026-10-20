# AmbWave v0.1 — UI and Performance Workflow Assessment

Date: 20 October 2026
Scope: Current working source, including uncommitted changes and the new DRONE role.
Method: Read-only source inspection. No implementation, build, fresh test run, or hardware operation was performed.

## Readiness verdict

AmbWave is suitable for a prepared, guided demonstration, but is not ready for the complete standalone workflow assessed here. The principal blockers are incomplete scene recall, conflicting mute systems, and transitions into inherited controls that can silently remove AmbWave behavior.

This assessment does not establish that the published v0.1.0 package contains the current working changes. “Verified” means traced through implementation. Existing tests provide supporting evidence, not fresh validation.

## 1. Current UI and control map

| Control | Current behavior | Qualification |
|---|---|---|
| ALGORITHM | Select tracks 1–4 | Selection itself does not release notes |
| PRESETS on ambient HOME | Immediately load a compatible sound | Clears that track’s held notes and resets macros |
| HOME + PRESETS | Immediately change role | Selects the new role’s first sound |
| Knobs 1–4 on HOME | Shape, Motion, Density, Space | Some factory sounds deliberately disable macros |
| HOME + knobs 1–4 | Levels for tracks 1–4 | Momentary mix mode |
| HOME + first four black keys | Toggle AmbWave track mutes | Retains held pitches |
| HOME + ARP | Cycle HOLD / SCATTER / SEQUENCE | DRONE rejects mode changes |
| HOME + SELECT | Clocked-mode speed | In HOLD, SELECT can instead change global tempo |
| HOME + ENV | CLOUD articulation length | Clocked CLOUD only |
| PLAY | Start/stop transport | Stopping leaves held drones sounding |
| FX + OCT− | Clear selected ambient track’s notes | Also resets performance FX controls |
| SEQ from HOME | Set SEQUENCE and open step editor | DRONE rejects the mode change, but editor navigation still proceeds |
| SAVE tap | Open USER sound-save page | Another SAVE tap reaches PROJECT |
| SAVE hold | Undo/redo the last supported load | Does not restore the live chord |
| EDIT / ENV / LFO / FX / SCL / GLO / ARP | Inherited editing pages and shortcuts | Semantics are not fully reconciled with ambient state |
| HOME hold | Mix on ambient HOME; system menu elsewhere | Same gesture has different purposes |

Sources: `firmware/src/ui_input.c:865`, `firmware/src/ambient.c:863`, `firmware/src/ui_layer.c:1`.

### Role and Sound

“Role” currently means a curated sound family plus behavior profile.

| Role | Compatible sounds |
|---|---|
| TIDE | BODY PAD, WARM PAD, SILK PAD, FM SOFT PAD, AN SOFT PAD, AN STRINGS, AN PWM STR, CHOIR AAH |
| TEXTURE | NOISE |
| CLOUD | GRAIN |
| PULSE | WARM PLUCK, SOFT MALLET, BELL PLUCK, GLASS BELL, FM MARIMBA, FM TINE EP |
| DRONE | AN SOFT PAD |

Any track can take any role, but arbitrary role/sound combinations are unavailable. Role and Sound are separately selectable, yet changing Role does not preserve Sound. There is no literal PAD role: TIDE is the pad family. These distinctions should be explicit in the product vocabulary.

Source: `firmware/src/ambient.c:25`.

## 2. Primary performance workflow

### Start fresh

Boot creates TIDE / TEXTURE / PULSE / CLOUD, with PULSE in SCATTER and the others in HOLD. Patterns start empty and transport stops. Track 1 is selected. DRONE is available but is not a startup role. Boot initializes this scene rather than automatically recalling the last configuration.

Sources: `firmware/src/main.c:100`, `firmware/src/ambient.c:886`.

### Choose a track, Role and Sound

ALGORITHM chooses the track. HOME + PRESETS cycles roles; PRESETS alone cycles compatible sounds. Both load immediately. Levels, AmbWave mute, pattern and timing state are largely retained, but live pitches clear and macros reset. DRONE additionally forces HOLD.

Choosing a replacement sound is an interrupting action, not seamless auditioning.

Source: `firmware/src/ambient.c:775`.

### Play an evolving Drone

Select DRONE, then press overlapping notes to build a chord. Releasing them leaves the chord sounding without PLAY. Once all keys are released, the next gesture replaces the previous harmony. Tapping separated notes therefore replaces notes rather than accumulating a chord.

This differs materially from TIDE HOLD, where successive taps add/remove individual pitches. DRONE provides slow filter, gain and fine-pitch movement plus Density-controlled detuning. Its random-walk intervals operate over tens of seconds, so movement may not be immediately apparent.

Sources: `firmware/src/ambient.c:524`, `firmware/src/drone.c:31`.

### Switch tracks and play Pad chords

ALGORITHM changes selection without clearing the drone. Physical key releases retain their original track ownership. Select TIDE on another track and tap a chord into HOLD.

All tracks share eight sounding voices. Four Drone notes plus four Pad notes already fill that budget before Texture or release tails. Track switching preserves ownership; it cannot guarantee every drone voice survives later allocations.

Sources: `firmware/src/ui.c:1083`, `firmware/src/seq.c:558`, `firmware/src/voice.c:181`.

### Add Texture

Choose another track, assign TEXTURE and tap a note. Texture holds one register; the same note toggles it off, while a different note changes register. The implementation attempts to reuse its existing voice to avoid unnecessary retriggering.

Source: `firmware/src/ambient.c:475`.

### Transform and mix

The four HOME knobs control the selected track. Hold HOME for four-track levels and use HOME + black keys for mutes. This avoids deep menus, although several actions require holding one control while reaching another.

### Change Role or Sound during performance

Other tracks continue, subject to shared voices and FX. The changed track starts without its previous held pitches. Undo can recover the sound configuration, but the player must re-enter the chord.

### Save and restore

SAVE → USER → PROJECT exposes inherited project saving. It stores substantial synthesis and sequencing data, but does not restore an AmbWave scene. Project load explicitly invalidates all ambient roles. The restored track loses ambient HOME/latch behavior and associated runtime macro state.

The workflow succeeds through live layering with qualifications, then fails at reliable configuration recall.

Sources: `firmware/src/project.c:708`, `firmware/src/project.c:817`.

## 3. Prioritized UX problems

| Priority | Problem and consequence | Source references |
|---|---|---|
| P0 | Project recall exits AmbWave behavior. Role, patch identity, macro values, dry balance and ambient mute are runtime-only. Engine parameters cannot reconstruct them reliably. | `seq.c:43`; `project.c:829` |
| P1 | Two independent mute systems. HOME toggles `ambient_muted`; GLO/MIXER changes `P_MUTE`. HOME displays only its own flag. GLO’s “unmute all” does not clear AmbWave mutes. | `ambient.c:186`; `ui_layer.c:318` |
| P1 | PRESETS changes scope outside HOME. Many advanced pages browse the general Felucca bank. A sound load invalidates the ambient role, removing curated controls and latch behavior. | `ui_input.c:734`; `ambient.c:124` |
| P1 | Four-macro promise is inconsistent. Factory TIDE sounds disable Shape, Motion and Density; factory PULSE sounds disable Shape and Motion. `FACT` marks a restriction, but main controls become inactive. | `ambient.c:49`; `ui_draw.c:691` |
| P1 | Advanced edits and macro values can disagree. Raw FX/LFO/engine edits change parameters without updating macro metadata; later macro movement can overwrite edits. | `ambient.c:236`; `params.c:343` |
| P1 | HOLD does not explain note-entry semantics. DRONE replaces harmony per gesture; TIDE toggles notes; TEXTURE holds one register; factory plucks can decay despite being held. | `ambient.c:542` |
| P1 | Overview activity can mislead. ON/highlighting follows stored pitch counts rather than sounding voices. Stolen or decayed voices can still look active; clocked rows show RUN from global transport. | `ui_graph.c:1407` |
| P1 | Recovery is obscure. Transport stop preserves drones. Selected-track clear requires FX + OCT− and also resets performance FX. No obvious dedicated all-track silence action exists in the principal workflow. | `seq.c:647`; `ui_layer.c:419` |
| P2 | DRONE can enter an irrelevant step editor. SEQ continues navigation after DRONE rejects SEQUENCE mode; keyboard behavior can differ from HOME. | `ui_input.c:1080` |
| P2 | HOME hold and SELECT are context-sensitive. HOME hold mixes on ambient HOME but opens settings elsewhere. HOME + SELECT on HOLD falls through to tempo control unless BPM lock prevents it. | `ui_input.c:869`; `ui_input.c:1177` |

All filenames in this table are under `firmware/src/`.

### Saving while a drone sounds

Flash saving is refused while transport runs, but the guard does not count an independently sounding HOLD drone as transport activity. The source documents that flash erase silences audio. Saving over a stopped-transport drone needs explicit hardware verification and a defined user-facing policy.

Source: `firmware/src/project.c:736`.

## 4. Macro consistency

For the original curated sounds, the vocabulary is broadly coherent.

| Macro | Musical interpretation across current profiles |
|---|---|
| Shape | Harmonic complexity, filtering, pluck articulation or grain character |
| Motion | Irregular tonal movement, per-note variation or grain movement |
| Density | Harmonic weight/spread, grain activity, crackle rate or scatter probability |
| Space | Local spatial sends and dry balance into shared FX |

Density has the widest meaning, but remains understandable as “more material/activity.” Space should communicate that effect algorithms and buses are shared. Factory macro locks are the larger consistency failure.

## 5. Felucca inheritance and conflicts

| Classification | Controls/pages | Assessment |
|---|---|---|
| Essential | Track selection, levels, canonical mute, note clear, sound selection, project storage, scale/chord helpers, basic FX access | Supports playing and recovering a scene |
| Useful advanced | Engine editing, envelopes, LFO/modulation matrix, voice allocation, pan, detailed shared FX, MIDI/clock/USB, user presets, granular/sample/slice facilities | Preserve synthesis depth underneath the performance surface |
| Useful optional composition tools | ARP parameters, STEP/PATTERN, phrases, probability, ratchets, automation, SONG | Valuable without making them prerequisites |
| Redundant/conflicting conventions | Independent mute flags; general preset loading under the same knob; USER save before scene save; raw ARP mode versus ambient mode; raw values versus macro metadata | Reconcile behavior rather than remove synthesis capability |

HOLD names three different concepts: ambient playing mode, a button gesture and a system-menu timing threshold. Advanced ARP settings can also diverge from the ambient mode displayed on HOME.

The dedicated instrument experience is fragile because advanced access can change the rules of the performance surface.

Sources: `firmware/src/params.c:343`, `firmware/src/ui_layer.c:1`, `firmware/src/menu_items.c:10`.

## 6. Minimal recommended UI design

Improve the existing HOME screen rather than replace it. It already renders all four tracks, role, mode, mute and basic activity. The selected sound appears in the footer.

The smallest useful design would:

- Keep four track rows, a strong selection marker and four fixed macro cards.
- Show selected Role + Sound together in a stable, readable location.
- Distinguish latched pitches, actual sounding activity, muted state and transport state.
- Show effective mute regardless of which interface changed it.
- Expose HOME combinations when HOME is held: levels, mutes, Role, mode and speed.
- Make scene save/recall the primary SAVE destination in AmbWave.
- Label general bank browsing as advanced and make leaving the curated profile explicit.
- Provide discoverable selected-track clear and all-track silence actions.
- Give factory auditions a clearly identified advanced status until their macros are designed and reviewed.

A small movement indicator could help DRONE’s slow evolution, but should follow real modulation state. Decorative animation would not resolve the activity ambiguity.

One-handed note toggling, track selection and macro adjustment are supported conceptually. Reliable one-handed Role selection, mixing and muting require physical testing of spacing, grip and encoder resistance. Inherited layer locking helps some advanced operations, but HOME mix is momentary.

Sources: `firmware/src/ui_graph.c:1407`, `firmware/src/ui_draw.c:568`.

## 7. Minimum persistence changes

A reliable scene record needs, per track:

- Stable Role and compatible Sound ID.
- Playing mode, four macro values, speed, explicit-speed flag and CLOUD articulation.
- Canonical mute and level.
- Existing engine parameters, FM patch, pattern and advanced edits.

Dry balance can be derived from Space. Restore must not reinitialize a preset over saved advanced edits: identity and behavior must be restored alongside the stored sound.

Live latched pitches need an explicit policy. A reasonable minimum is silent restoration of the complete configuration, with that behavior stated clearly. Exact resumption of pitches or random-walk phase is a separate feature.

Storage work must include versioning, validation, legacy import, missing-metadata behavior and torn-write recovery. Keep the existing loader, recovery path and firmware package format unchanged. Do not assume existing reserved project bytes can hold a complete scene extension.

The factory test explicitly expects project recall to leave curated mode. Present coverage therefore validates source preservation rather than complete ambient scene recall.

Sources: `firmware/src/project.c:708`, `firmware/src/project.c:817`, `tests/factory_audition_test.c:98`.

## 8. Release blockers versus optional polish

### Blockers for the requested workflow

- Reliable AmbWave scene round-trip.
- One effective mute model with truthful feedback.
- Predictable advanced-sound access without silently losing ambient behavior.
- Clear note-entry, stop and recovery behavior.
- Hardware validation of Drone + Pad + Texture under voice pressure, save interruption and target CPU behavior.
- Clear product promise for factory sounds whose macros remain inactive.

### Optional polish

- New artwork, animated backgrounds and more palettes.
- Elaborate meters or movement graphics.
- Additional sounds.
- Exact evolution-phase recall.

## 9. Proposed implementation sequence

Estimates exclude hardware feedback cycles.

| Order | Work | Effort | Main regression risk |
|---|---|---|---|
| 1 | Define Role/Sound/mode terminology, HOLD semantics, mute and recall policy | ½–1 day | Existing user expectations |
| 2 | Unify mute state and correct overview feedback | 1–2 days | Solo, FX mutes, new-note behavior and persistence |
| 3 | Implement versioned scene persistence and restore | 3–5 days | Legacy projects, storage capacity, advanced edits and recovery |
| 4 | Make HOME/advanced transitions predictable; address DRONE → SEQ and recovery gestures | 2–3 days | Tap/hold timing, shortcuts and accidental loads |
| 5 | Refine HOME labels, hints and SAVE routing | 1–2 days | Small-display layout and LARGE mode |
| 6 | Review factory macro profiles or clearly separate auditions | 2–4 days | Intentional timbre changes and audio regressions |
| 7 | Run integrated software checks and physical workflow trials | 2–3 days | Target timing, voice stealing and audible discontinuities |

Validation should cover scene round-trips, legacy storage, physical-key and MIDI ownership, every mute entry point, advanced-edit/macro interaction, UI layouts, target CPU budget and reviewed audio regressions.

Existing Drone tests cover several ownership and evolution cases. The ambient UI sweep currently rotates the original four roles and omits DRONE.

Sources: `tests/drone_test.c:15`, `tests/ambient_ui_test.c:18`.

## 10. What still requires hardware testing

- Physical one-handed reach for HOME combinations, clear gestures and track changes.
- Display readability at performance distance, including long names and LARGE mode.
- Audible discontinuities during sound/role changes, mute transitions and flash saving.
- Drone movement audibility over realistic listening intervals and output devices.
- Voice stealing during Drone + Pad + Texture, including release tails.
- Worst-case CPU behavior on the FM-1.
- Full save, power-cycle and recall behavior after persistence work.

For a demonstration today, use a rehearsed live setup, keep chords modest, choose sounds before starting each layer, and demonstrate clear/mute explicitly. Do not present project recall as complete scene recall.
