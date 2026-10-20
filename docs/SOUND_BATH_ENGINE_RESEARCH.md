# Sound Bath Box — engine and preset discovery

Research date: 2026-10-20. Research only; no firmware implementation, package, hardware installation or new repository.

## 1. Executive summary

**Use Felucca FM6 as the first audition platform, with PHYS as the essential comparison. Select the final engine by listening before choosing the new repository's foundation.** Begin with the actual existing patches GLASS BELL, SOFT PAD, BELL TREE, BOWED METAL and DRONE STRING. These are candidates, not accepted sounds. No candidate has yet demonstrated the proposed complete Sound Bath Box experience.

The most promising division is FM6 for playable bells and a supporting drone, with PHYS MODAL available if its sustained metal is clearly better. A secondary engine is not necessary for the first prototype: audition both separately, then include the second only if it wins a distinct musical role. Do not build another pad from scratch.

For the shortest route to a sound proof, reuse a pinned Felucca source snapshot and its existing engine/preset host or emulator. For the eventual focused firmware, choose between a reduced Felucca application and a new application on felucca-bsp/fm1-base only after this proof. A fresh platform application is architecturally attractive but carries more integration work. AmbWave's four-track product model is not a requirement.

The concept is an electronic resonance instrument for personal listening and playing. No therapeutic outcomes or special healing-frequency claims are part of this recommendation.

## 2. Evidence and present AmbWave status

Local HEAD inspected: `828ceaa3fd0b62e825820998e1a09ab90d6edcd1`. The checkout has substantial existing uncommitted work; findings describe the inspected working files, not a clean release or a claim about everything currently installed on hardware. Only this report was added for this task.

Local sources: [current plan](../AMBIENT_PLAN.md), [README](../README.md), [FM6](../firmware/src/eng_fm6.c), [factory FM patches](../tools/gen_fm6_patches.py), [PHYS](../firmware/src/eng_phys.c), [voice limits](../firmware/src/core.h), [graphics](../firmware/src/gfx.c), [LCD](../firmware/src/lcd.c), [CPU reference](../tests/cpu_baseline.txt).

AmbWave currently separates roles from compatible sounds and offers HOLD/SCATTER/SEQUENCE, four musical macros and a direct mixer. Its sources include FM6 TIDE, analog WARM PAD, FM6 SILK PAD, noise, granular and plucks. Eight voices and spatial buses are shared; full curated-scene persistence is absent. Those are useful lessons, not a proposed Sound Bath Box feature list.

[Prior listening evidence](AMB1_DRY_REFERENCE.md) says the user preferred the fullness of AMB-1's isolated Rhodes and Voices references, with Rhodes more grounded. [FM audition notes](TIDE_FM_PATCH_AUDITION.md) record the subsequent FM direction as closer. Later [plan notes](../AMBIENT_PLAN.md) distinguish movement from richness and show why further parameter invention should wait for listening.

Evidence labels in this report:

- **Verified local:** source or recorded results inspected in this checkout; not a new hardware measurement.
- **Upstream documented:** linked project materials, which may move independently of this checkout.
- **Inference/proposal:** technically plausible, still requiring validation.
- **Unverified:** no listening or measurement supporting acceptance.

No builds or regression tests were run for this documentation-only task. The unchanged upstream build/test proof already exists in AMBIENT_PLAN.md; it must be repeated for whichever exact source revision starts the future project.

## 3. Engine and foundation comparison

| Candidate | Capabilities and presets | Sustain and runtime control | Resource evidence and integration | Verdict |
|---|---|---|---|---|
| Felucca FM6 | Six operators, 32 algorithms, ratio/fixed frequencies, per-operator envelopes, feedback, pitch envelope and LFO. Eight local factory voices; standard DX7 voice data. | Operator envelopes govern lifetime. Live brightness/index paths exist; a held key does not stop a patch whose sustain levels decay to zero. | Local cap six voices per part, eight globally shared. Integer msfa port; existing editor, project and user-preset infrastructure. | Primary audition engine; broadest patch reuse route. |
| Felucca PHYS | Modal resonators, strings, membranes and sympathetic strings. Existing metal, bell and drone presets. | MODAL has 12 modes, damping and continuous excitation via BOW. Local comment describes this as random excitation, not a detailed friction-bow simulation. | Three voices per part locally. Larger per-voice state pool; fixed-point DaisySP/Rings-derived DSP. | Strongest alternative for Infinite Bowl. Audition before committing to FM-only. |
| Felucca ANALOG | Established oscillator/filter engine and factory presets; sustained low drones feasible. | Conventional envelopes and filter/modulation controls. | Existing integration; shares global voice/FX budget. | Optional fallback for Deep Earth if FM pad fails listening; no need to add it preemptively. |
| Felucca DIGITAL/FM4 | Legacy four-operator FM presets including BELL. | Different parameter model from DX7. | Disabled by default in newer upstream; migration to FM6 exists. | Do not revive solely for this project or treat DX7 banks as directly compatible. |
| AMB-1 | Purpose-built ambient synthesis and curated whole-instrument recipes. Prior local audit identifies Rhodes FM and a formant Voices source. | Its existing generative behavior differs from direct keyboard strikes/latching. Porting an isolated source requires separating that behavior. | Known FM1 deployment; no comparable native CPU/RAM measurement collected here. Host/emulator tooling exists. | Musical reference and bounded component candidate, not an automatic whole-firmware fork. |
| Official FM1 Open V13 | DX7/FM plus 303, 808, PSG, pluck and sample families; official workbench. | Prior pinned SDK inspection reports FM patch edits affecting subsequent notes; held-note macro requirements need explicit native testing. | Native instrument is binary; browser synthesis is open source. No verified custom native FM-instance or reverb API. | Do not select it on the assumption that an official SDK improves sound or live patch editing. |
| New app on felucca-bsp / fm1-base | Reuses platform and host/browser infrastructure while leaving musical design open. | Application must supply engine hosting, performance state and visuals. | BSP supplies no synth/UI. Beta interfaces; additional integration and regression burden. | Plausible final home after sound validation, not the fastest audition vehicle. |

Upstream distinction: [Felucca main](https://github.com/hugelton/Felucca) currently describes 1.6.1. Its [core definitions](https://github.com/hugelton/Felucca/blob/main/firmware/src/core.h) retain DIGITAL as a compatibility identity with conversion. Newer bank-loading/UI features must not be assumed present in this older local fork.

The separately published [Felucca-Engines](https://github.com/hugelton/Felucca-Engines) offers extracted synthesis components, but its advertised scope does not establish an extracted FM6/PHYS package. It is not a drop-in replacement for this recommendation. [felucca-bsp](https://github.com/hugelton/felucca-bsp) and [fm1-base](https://github.com/charlesvestal/fm1-base) are platform alternatives, not preset libraries.

Official evidence: [FM1 Open](https://github.com/m-vave/fm1_open), and the detailed local [SDK assessment](research/FM1_OPEN_SDK_ASSESSMENT.md), pinned to V13 `b178ac43c25f010c60a878056fb5331fe693fd62`. This research reused that existing source audit and inspected the retained SDK directory; it did not repeat its native binary analysis. Browser capacities are not simultaneous hardware-polyphony promises.

## 4. Concrete patch shortlist and reuse

All five first-round candidates below exist in local source. Source-defined patches are directly reusable with their existing engines; they are not interchangeable across synthesis families.

| Actual patch | Location / format / loading | Intended audition role | What is still unknown |
|---|---|---|---|
| GLASS BELL (device preset BELL) | `tools/gen_fm6_patches.py`, second factory voice, packed 128-byte DX7 voice generated into FM6_FACTORY; device BELL selects factory slot index 1. | Crystal Garden; struck reference for Infinite Bowl. | Whether its transient is pleasant and its dry tail rich enough. Its current carrier envelopes decay toward zero: latching alone is not infinite sustain. |
| SOFT PAD (device preset PAD) | Same generator, fifth factory voice; existing FM6 preset loader. | Deep Earth; supporting drone for Floating Temple. | Low-register body, masking and long-hold fatigue. Existing nonzero sustain makes it a better indefinite-drone starting point than BELL. |
| BELL TREE | `firmware/src/eng_phys.c`, PHYS_PRESETS parameter record; existing factory loader. | Crystal Garden and bowl comparison. | Whether the modal spectrum reads as bowl/chime rather than generic metallic percussion. |
| BOWED METAL | Same PHYS table; MODAL with BOW excitation, high damping parameter and track sustain. | Infinite Bowl. | Noise content, steady-state level, fatigue and behavior under repeated strikes. |
| DRONE STRING | Same PHYS table; sympathetic-string model and parameter record. | Deep Earth alternative; Floating Temple's foundation. | Whether the resonant string identity suits the product and how long useful sustain lasts. |

Source links: [FM factory generator](https://github.com/mentalcells/ambwave/blob/main/tools/gen_fm6_patches.py), [PHYS presets](https://github.com/mentalcells/ambwave/blob/main/firmware/src/eng_phys.c). Local files above are the actual inspected authority; public main may differ.

Floating Temple is a **proposed scene**, initially SOFT PAD plus GLASS BELL, not an existing discovered preset. Likewise Infinite Bowl, Deep Earth and Crystal Garden are product intentions rather than claims about a library.

### Additional existing libraries

DX7 ROM1A's actual named voices include **ORCH-CHIME (25), TUB BELLS (26), VIBE 1 (21), E.PIANO 1 (11)**. The [dx7dump author's example](https://github.com/lexonic/dx7dump) identifies these bank positions; [sevenator](https://github.com/coniferprod/sevenator) documents the voice/bank format. These are useful external comparison targets, not an approved factory payload. Public availability of a bank does not establish redistribution rights to its patch data. Obtain a permitted bank and record provenance before importing or shipping it.

Standard DX7 data: 128 packed bytes per voice; 155 unpacked parameter bytes; commonly 163-byte single-voice SysEx and 4,104-byte 32-voice bank SysEx. Felucca FM6 uses this family of data, rather than manual parameter recreation. Local FM6 supports editor-to-track voice loading and stores its own patch in projects/user presets. The local fork retired its earlier flash bank slots; use single-voice import through the matching editor protocol or extract a bank voice offline. New upstream releases restore a bank facility; adopting it is separate work. Never send a hardware import during this research.

A further FM1-native comparator is [sloopDX](https://github.com/zvenson/dxsloop/blob/main/SLOOP.md): it documents original BELLS and GLASS PAD voices and DX7 bank import. Its compatibility is promising; exact data availability, license notices, runtime edits and performance have not been audited here. It is a second-round candidate rather than a reason to change the foundation now.

AMB-1's **Garden/Departures** (Rhodes support) and **Glass/Shoreline** (bell contrast) are documented whole-instrument recipes. They are not DX7 banks: reuse requires source-level extraction of recipe and engine behavior, not importing `.syx`. See its [manual](https://github.com/charlesvestal/fm1-amb/blob/main/docs/MANUAL.md). Keep the previously auditioned Rhodes revision pinned if comparing it with Tide; current AMB-1 has changed platform and licensing claims.

## 5. Sound assessment and reproducible listening gate

**No new subjective sound-quality judgment was made by the agent.** Existing user feedback supports AMB-1 Rhodes/Voices as fullness references only; it does not accept the five shortlisted patches as beautiful bowl sounds.

The current [AMB-1 browser emulator](https://charlesvestal.github.io/fm1-amb/emu/) was opened and switched on successfully. The UI reported On, Garden, D Dorian and growing. This verifies an audition route, not that audio was heard, captured or evaluated. Its displayed computer CPU percentage is not a hardware benchmark. No new native or browser audio captures were made.

Reproducible next listening session, before implementing the product:

1. Pin each source revision, patch bytes/record and effects configuration. Use the matching existing Felucca browser/host renderer for the five candidates; use AMB-1's existing host recipe for its reference. Do not alter production defaults.
2. Render each candidate dry: C3, C4 and C5 at fixed MIDI velocity 80, eight seconds held then 30 seconds released. Add C2 and G2 for drone candidates. Record exact note numbers/octave convention so labels cannot shift the comparison.
3. Render a 60-second held note for sustain candidates, repeated strikes while a tail is active, a two-note close interval for beating, and a sparse root/fifth scene. Fix random seeds where supported. Preserve each original patch first; any envelope edit becomes a separately named variant.
4. Match playback loudness using one fixed gain per take, preserve raw originals and report peaks. Listen dry first, then with the same available reverb settings where possible. Keep effects from disguising a weak source; compare native effects separately when different engines require different hosts.
5. User rates attack softness, body, metallic character, high-frequency harshness, natural tail, useful beating and fatigue. Compare blind where practical. A synth spectrum is not a substitute for this assessment.
6. During held notes sweep only proposed continuous controls. Listen and inspect recordings for discontinuities. Repeat with maximum voices, FX and graphics enabled later on hardware, with explicit installation approval.

Acceptance: at least one struck sound and one sustainable sound must be compelling dry, then remain controllable with space added. If they fail, broaden existing-preset discovery before writing DSP.

## 6. Proposed minimal architecture

Use a small static table and direct dispatch, not a plugin framework:

- **Engine:** existing FM6, optionally PHYS; owns synthesis state, voice completion and engine limits.
- **Patch:** immutable source data plus a small mapping record. Keep engine-specific bytes intact; distinguish base patch from temporary performance offsets.
- **Performance:** keyboard strikes, hold/latch state, release/clear and optional sparse note scheduling. It sends ordinary note events; it does not define oscillator topology.
- **Scene:** optional one drone patch plus one strike patch, fixed levels and behavior settings. Start with one patch at a time; add this only after each source passes listening.
- **Visuals:** read a bounded event queue and low-rate snapshots of active pitch, strike strength, envelope/energy and macro values. No DSP or UI drawing in the other's critical path.

Proposed common knobs preserve familiar meanings, with engine-specific bounded mappings:

| Macro | FM6 proposal | PHYS proposal |
|---|---|---|
| SHAPE | Modulator level/brightness in a curated range | Brightness and strike position |
| MOTION | Slow, shallow index evolution; optional subtle beating | Slow brightness/position movement |
| DENSITY | Harmonic weight of existing operators; later sparse-note activity only in an explicit generative mode | Continuous excitation amount, with level compensation established by listening |
| SPACE | Existing reverb/delay sends and dry balance | Same musical intent and FX routing |

Avoid changing algorithm, coarse frequency ratios or entire patch bytes continuously during a held note. Local FM6 rebuilds an effective patch at control boundaries and has gain/index ramps; that is evidence for selected live control paths, **not proof that every edit is click-free**. Envelope changes, feedback, frequency changes and patch replacement require separate tests. PHYS coefficients and excitation changes also need sweep testing.

For Infinite Bowl, distinguish a long decaying strike, a continuously excited body and a frozen effects tail. They sound and respond differently. Prefer PHYS's existing continuous excitation or an explicitly sustained FM variant over claiming that an unchanged decaying bell becomes infinite when held. Any revised FM envelope is a deliberate patch edit requiring listening approval, not custom DSP.

## 7. Graphics feasibility

Verified local LCD driver: ST7789-class **240×240 RGB565**, SPI DMA, default 12 MHz. A full RGB565 frame is **115,200 bytes**; two are 230,400 bytes. Felucca avoids a full framebuffer: `CV_MAX=240×124` gives **59,520 bytes** of reusable strip canvas. Pixel data must remain valid until DMA finishes; `lcd_sync()` guards buffer reuse.

Ideal full-screen transfer alone takes 76.8 ms at 12 Mbit/s, or about 13 frames/s before commands, drawing, contention and other work. A 240×124 strip takes about 39.7 ms, suggesting an ideal ceiling near 25 strips/s. These are calculated bandwidth limits for this driver setting, not measured animation rates or limits for every possible LCD configuration.

**Proposal:** rings and overlapping circles in a bounded central area at an initial 10–15 Hz target, static labels updated only when changed. Measure before increasing rate. Use existing canvas blending, lines, font/icon assets and DMA blits; no external graphics framework is necessary. Integer circle spans or a small lookup table should suffice. No per-pixel blur, full-screen feedback texture, large particle system or new framebuffer is needed for the first visual proof.

A strike launches a ring; its radius follows visual time and opacity follows an envelope/energy estimate. A sustained voice maintains a slowly moving contour. Multiple notes can overlap within a fixed maximum ring count. If rings are driven by note events alone, describe them as event-reactive; obtain actual voice/envelope telemetry before calling them audio-reactive. A short peak/RMS snapshot may later describe the shared effects tail without exporting audio buffers.

Animation runs in the main/control loop, reads stable snapshots and may skip frames. Audio never waits for drawing. Existing UI rendering hooks can measure pixel/blit cost, but neither browser smoothness nor ideal SPI math proves absence of audio overruns. A worst-case hardware trace with graphics on/off is required. AMB-1's existing animated musical display provides a useful feasibility reference, not a performance guarantee for a different renderer.

## 8. Constraints, measurements and licensing

Local FM6 state is documented as 244 bytes per note, with six state slots per part; four parts imply roughly 5.9 KB for that side array, excluding effective patches, voice objects, FX and other state. PHYS reserves 12 model slots; its largest model includes 512+128 Q20 string samples and three 256-sample 16-bit sympathetic lines per slot. Confirm actual `sizeof`, linker pool allocation and stack margin before budgeting a reduced build. Do not infer free RAM from removing sequencer UI alone.

The retained host baseline lists FM6 BELL 1376, PAD 1475, PHYS BELL TREE 1347 and BOWED METAL 1451 host instructions per sample for its test scenarios. These are existing regression indicators, not milliseconds, current measurements, PI32 cycle counts or a controlled per-voice comparison. Actual native CPU, worst render time, free RAM and all-engine mixed polyphony remain unmeasured in this research.

Other required checks: release-tail stealing, resonator buildup, summing headroom, FX feedback stability, long sessions, sustained-note clearing, repeated strikes, patch transitions, held-note macro sweeps, deterministic regression renders, storage compatibility and visual/audio contention. Preserve bootloader, loader, recovery contract and package format. Read user data only when needed; no cross-firmware storage migration or preset overwrite is authorized.

License evidence, not legal advice:

- Local Felucca/AmbWave application and factory assets are GPL-3.0-only; local msfa core is Apache-2.0; DaisySP/Rings-derived DSP retains MIT notices. See [LICENSING.md](../LICENSING.md). Importing GPL wrappers/presets is different from importing an independently permissive DSP file.
- [Upstream licensing](https://github.com/hugelton/Felucca/blob/main/LICENSING.md) distinguishes application and extracted components. Confirm the exact files/revisions selected before distribution.
- [BSP README](https://github.com/hugelton/felucca-bsp) documents MPL-2.0 and expressly says using the BSP does not relicense a Felucca fork. A new repository is not itself a license reset.
- AMB-1 current README says MPL-2.0 with older releases GPL; the licensing page returned during research still describes GPL-3.0-only. Treat this as unresolved snapshot/documentation inconsistency. The previously auditioned `d9ef0a35823bbad8edc7cc0feb2eae59539e4081` is recorded locally as GPL-3.0-only. Verify commit-level headers and notices before importing newer code. [README](https://github.com/charlesvestal/fm1-amb), [licensing](https://github.com/charlesvestal/fm1-amb/blob/main/LICENSING.md).
- Official application licensing, native SDK redistribution/linking exception and third-party synthesis terms are separate. Use the pinned [SDK assessment](research/FM1_OPEN_SDK_ASSESSMENT.md) and [official notices](https://github.com/m-vave/fm1_open/blob/main/THIRD_PARTY.md); do not treat the binary renderer as unrestricted source.
- External DX7 patch banks need their own provenance/redistribution decision; the engine's license does not license every compatible preset.

## 9. Recommended first playable prototype — future work only

**First gate: five unmodified patches, dry and with space, using existing audition infrastructure.** No custom interface is needed to decide whether the source sounds earn further work.

Once that gate passes, the minimum playable build is:

1. One selected existing engine and two or three accepted patches; direct keyboard strikes, optional latch, clear/release.
2. Four constrained macros applied as offsets to accepted base patches; a single existing spatial chain with measured headroom.
3. One central event/envelope-driven ring visualization, stable labels and a bounded refresh rate. Visual identity is part of the prototype, following source acceptance.
4. A fixed voice policy: conservatively budget no more than six FM voices, or three PHYS voices per part in the current host. For a two-patch scene initially reserve space for tails rather than filling all eight voices with drones.
5. Build and run relevant host tests, regression renders, CPU-budget checks, storage compatibility and UI tests; then native worst-case profiling and user listening. Do not change golden audio hashes without an intentional reviewed sound change.

No sequencer, patch browser, broad preset bank, custom DSP, new recovery mechanism or complete scene format is required for this gate. Optional generation follows manual playability. A scene must justify its extra voice/routing complexity by sounding better than a single accepted patch.

## 10. Decision still requiring hands-on evidence

- Does PHYS BOWED METAL beat FM6 as the defining bowl sound? Is its continuous excitation pleasant over minutes?
- Which actual patch wins dry, in which register and with which release duration?
- Can an accepted FM strike be edited into useful continuous resonance without losing its identity?
- Which exact live parameters remain smooth across the full intended macro range?
- What native voice/FX/animation budget leaves adequate worst-case margin?
- How many overlapping tails feel playable before stealing is distracting?
- Does a shared drone/bell spatial chain preserve clarity and level?
- Which source revision and file licenses should the future repository pin?

**Recommendation to carry forward:** shortlist existing FM6 and PHYS sounds now; choose by a small listening session; create the new firmware repository only after a struck and sustained source are accepted. Reuse synthesis, curate patches, and put original work into playing behavior and visual identity.
