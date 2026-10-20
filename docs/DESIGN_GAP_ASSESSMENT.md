# AmbWave: architecture assessment and sound improvement plan

Assessment date: 2026-10-18. Source: `828ceaa` on `codex/ambwave-mvp`
(also the current `ambient-prototype` tip). **Investigation and proposed work;
no DSP, controls, stored formats or factory sounds changed by this assessment.**

## Recommendation

2026-10-18 follow-up: the user approved **only an isolated, opt-in P0a
experiment**, with factory Tide unchanged pending listening acceptance. Judge
static fullness separately from movement and stop tuning breathing if the
first result is unconvincing. After P0a, bring forward an assessment of cheap
stereo decorrelation within the existing network. Reliable performance
save/restore is a separate, higher priority than morphing; keep coherent
four-layer sound curation and level balancing in scope throughout. The
original priority labels below are retained for traceability, subject to
this revised order. See [P0a results](TIDE_P0A_RESULTS.md) for implementation,
comparisons, measurements and the listening decision still pending.

Keep FM6 as the main Tide architecture for the next milestone. First test
whether independent, gentle amplitude/index breathing makes the existing dry
FM patch feel fuller and more alive. Keep its stable pitch, existing Density
control and four-note latch. Compare against the current sound with matched
notes, register and loudness before changing the default.

The source already provides several synthesis architectures, detuning, smooth
random modulation, a modulation matrix, chorus, two reverbs, sample import and
a direct four-track level mixer. The central gaps are **curation and coordinated
movement**, **per-track frequency cleanup**, and **stereo spatial processing**.
Adding engines or controls does not by itself address these gaps.

This recommendation incorporates prior listening: the saw/triangle dry-source
experiment and independent harmonic exchanges were different but not clearly
richer; the AMB-1 dry references were fuller; the integrated FM audition got
closer. There is still no evidence that the current Tide has met the requested
dry-sound quality bar. See [listening history](#listening-evidence-and-causal-limits).

## 1. Current instrument and signal path

The four names describe curated **sounds on interchangeable slots**, rather
than four hard-wired engines. Any slot can select Tide, Texture, Cloud or Pulse.
Boot order is **Tide / Texture / Pulse / Cloud**, with levels 82/80/94/80
(parameter values, not linear percentages). Tide, Texture and Cloud boot in
HOLD; Pulse boots in SCATTER. HOLD, SCATTER and SEQUENCE are independent of
sound choice. Changing sound resets that slot's notes but preserves its level,
mute and playing-mode choices.

HOME's actual knob order is **Shape / Motion / Density / Space**. The conceptual
order used in some descriptions should not cause a physical-control change.
HOME held gives four simultaneous track faders; HOME plus the track keys gives
layer mutes. A conventional selected-track mixer also remains in the full UI.

The firmware is assembled in [felucca.c](../firmware/src/felucca.c).
The relevant path is:

```text
panel latch / MIDI / arp scatter / sequence
  -> shared voice allocator and per-track modulation
  -> engine block hook and mono voice rendering
  -> track distortion -> slicer / temporary mute
  -> track level -> mono chorus / delay / reverb sends
                -> Space dry trim -> track pan -> L/R dry mix
  -> identical mono wet return added to L and R
  -> optional master performance effects
  -> master DC blocker / optional speaker processing / limiter / soft clip
  -> DAC and USB audio
```

Space trims only dry perspective, not the signal feeding the sends. Track
level affects both dry and sends; pan affects dry, not the mono effect sends.
The master limiter is shared, so one loud layer can reduce the apparent level
and dynamics of every other layer.

Source anchors: [ambient.c](../firmware/src/ambient.c),
`ambient_sound_apply`, `ambient_boot_apply`, `ambient_latch_toggle`;
[fx.c](../firmware/src/fx.c), `mix_part`, `fx_buses`, `mix_block`;
[ui_input.c](../firmware/src/ui_input.c), HOME mixing and `tracks_edit`.

### Track architectures and macro behavior

| Sound | Actual source and primary behavior | Shape / Motion / Density / Space |
| --- | --- | --- |
| **Tide** | FM6, custom `TIDE BODY` patch, algorithm 5; one 1:1 FM pair and a quiet unmodulated second harmonic. FM operator envelopes sustain held notes. | Modulator level / shared DRIFT into FM index / second-harmonic gain / delay and reverb sends plus dry perspective. No chorus at any setting. |
| **Tide Warm** | Existing ANALOG two-oscillator saw pad, resonant low-pass, drive, long common ADSR. | Cutoff plus drive / DRIFT into cutoff / oscillator blend and detune plus conditional chorus / spatial sends and dry trim. |
| **Tide Silk** | Existing FM6 PAD-derived three-pair patch, carrier levels adjusted for sustained body, pitch LFO depth disabled. | Modulator level / index DRIFT plus a higher index centre at high Motion / carrier pitch spread plus conditional chorus / spatial sends and dry trim. |
| **Texture** | NOISE in analog-noise mode; white/pink/brown source tilt, random impulses and resonant low-pass/band-pass blend. One panel-latched register in HOLD. | Source colour and frequency / shared DRIFT into filter / impulse rate, up to about 32/s / delay and reverb sends, dry trim. |
| **Cloud** | GRAIN over the shared sample infrastructure; boot source 2 is FLUTE in this build. Up to three panel-latched notes in HOLD. | Grain size plus tone and density compensation / source-position DRIFT and spread / grain trigger density / shared sends and dry trim. |
| **Pulse** | PHASE, not a new percussion engine: phase-distortion plucks with SAW, DSIN and resonant TRI timbres. SCATTER draws from up to four held pitch choices. | Wave bending and envelope / per-note random tone, amplitude and track pan / event probability; in other modes also some detune/chorus / sends and dry trim. Speed is separate from Density. |

All sounds can use the existing sequence editor. The firmware already contains
a 64-step sequencer; no sequencer implementation is needed for this plan.
Cloud's SHORT/FLOW/LONG choices alter clocked envelopes and gates; HOLD remains
sustained. A held pitch choice is not a guaranteed permanently sounding voice:
all layers and release tails compete for the same voice budget.

Sources: [ambient.c](../firmware/src/ambient.c), `ambient_*_apply`,
`ambient_mode_apply`, `ambient_pulse_timbre_apply`;
[eng_phase.c](../firmware/src/eng_phase.c),
[eng_noise.c](../firmware/src/eng_noise.c),
[eng_grain.c](../firmware/src/eng_grain.c).

### Tide in detail

`ambient_tide_patch()` sets algorithm byte 4 (display algorithm 5), feedback
zero, operator sync on, and patch pitch/amplitude LFO depths zero. Three
operators have nonzero output levels:

- OP1: fundamental carrier, level 99, sustained envelope.
- OP2: same-frequency modulator, level 66, a slower attack to bloom after onset.
- OP3: unmodulated carrier at twice the fundamental, level 75.

The remaining operators are silent. Shape moves `P_E2` through -16/0/+12
at 0/50/100%. This changes FM index, **not a low-pass filter**. The track
destination named FLT also maps to FM modulator level in this engine. Motion
sets this one shared DRIFT destination, with depth 0/10/24 at 0/25/100%.
Pitch, amplitude and feedback modulation depths are zero.

Density applies a live log-gain offset to the second harmonic, approximately
-12/0/+12 dB at 0/50/100%. It adds neither voices nor oscillators. The offset
uses the FM core's gain ramp and preserves its silent-envelope floor.
Default Shape50/Motion25/Density50 reproduces the prior FM audition.
Space0 clears all three sends; original Tide keeps chorus off even with Space
raised. Clear old bus tails when making a dry comparison.

The resulting sustained chord is relatively coherent: no detuned carrier bed,
no per-note evolving balance, and one modulation trajectory shared by all
notes. This is a plausible reason for a less enveloping result, **not proof
that FM itself is thin**. Aggressively raising index can redistribute energy
away from the fundamental; raising a second harmonic can add brightness or
an octave impression without creating the desired body. Extra gain can also
activate the master limiter. Measure these separately from perceived fullness.

FM6 owns its six operator envelopes; common track ADSR and ENV matrix routing
do not provide its ordinary envelope control. Patch level/rate changes are
largely prepared at note-on. A new control must be tested on already-held notes;
changing a patch byte alone is insufficient evidence of live behavior.

Sources: [eng_fm6.c](../firmware/src/eng_fm6.c), `fm6_sync`, `fm6_render`;
[fm6_core.c](../firmware/src/fm6_core.c), `fm6_note_init`,
`fm6_note_compute`; [tide_fm_test.c](../tests/tide_fm_test.c).

## 2. Existing capabilities versus actual gaps

| Area | Already implemented | Missing or unproven |
| --- | --- | --- |
| Oscillators | ANALOG has two oscillators, noise, drive and a trapezoidal low-pass. FM6 has six operators, 32 algorithms, detune, key scaling and operator envelopes. PHASE has eight bend shapes, a second line and sub. VOICE offers formants. | No high-quality evolving wavetable pad architecture. Existing LOFI wave RAM is a small quantized table source, not an equivalent. More importantly, the main Tide recipe underuses the existing FM topology and note dynamics. |
| Movement | One common LFO per track, including eased random DRIFT with variable segment timing; four matrix slots per track; velocity/key/random/MIDI sources. FM6 also has a patch LFO; NOISE and GRAIN have internal random processes. | No coordinated, independently timed per-note amplitude/index movement in Tide. Four matrix routes using one LFO remain correlated; note-on RAND does not keep evolving during a held drone. A patch LFO exists but current Tide disables its depth. |
| Filtering | Engine low-pass/tone filters, NOISE colour/resonant filtering, per-track distortion with low cut, master DC removal and optional speaker low cut; temporary master HPF/LPF. | No independent per-track mix HPF or low/mid tone controls. FM6's FLT-labelled destination is not an audio filter. Master low cut cannot selectively clean Texture/Cloud. |
| Space | Shared chorus, tempo delay, ROOM and SPRING. Performance OCT UP/DN includes feedback shimmer. Space is already a curved multi-parameter macro. | Main buses are mono. The performance shimmer is a temporary master buffer effect, not a dedicated per-track shimmer reverb; it borrows slicer storage. Reverb diffusion/decay quality at ambient settings needs listening and impulse-response evidence. |
| Mix | Four simultaneous HOME faders, layer mutes, selected-track pan/reverb/mute, level smoothing and a limiter. | Proposed cross-track frequency rows; transparent frequency cleanup before sends; reliable pre-limiter headroom evidence under sustained mixes. |
| Library | Three Tide colours, three Pulse timbres, built-in sample sets, 32 user-preset slots, three user-sample slots and editor upload validation. | A small, level-balanced set designed as coherent four-layer combinations. Texture is synthesized noise today; replacing its source with samples would be a product change, not merely supplying better files. |
| Scenes | Projects store instruments, FM patches, patterns and automation; older project formats import. | AmbWave scene identity/macros/latches/modes and 15–60 s continuous morphing are not saved as an ambient scene. Project load is not a scene morph. |

### Why Texture can become muddy

Texture starts with low-frequency-weighted noise (Shape35 gives colour 76,
beyond the pink midpoint) and filter frequency 63, low resonance 18 and key
tracking 24. Its built-in NOISE drift is disabled, but the common filter DRIFT
is active. The source/filter makeup gain can preserve substantial low energy.
Cloud's sampled flute body and all tracks' reverb sends add further overlap.
There is no independent post-engine track HPF. These are code-supported
suspects; the relative contributions still require isolated spectra and
layered listening. Reverb damping is high-frequency absorption, not selective
low-frequency cleanup for these tracks.

## 3. Constraints and performance risks

| Constraint | Evidence and consequence |
| --- | --- |
| Timing | Nominal 44.1 kHz, 32-sample control blocks (~0.726 ms), 128-frame DMA halves (~2.90 ms). Control-rate work is preferable to more per-sample work. |
| Polyphony | Eight sounding voices globally; FM6 cap six per track, GRAIN/PHYS three. Default four-note Tide plus Texture1 plus Cloud3 already consumes eight, before Pulse or tails. Unison would make this contention worse. |
| Overload policy | `shed_check()` requests shedding after two halves over 85% of the half-buffer time; `shed_voice()` fades a selected voice. Runtime timing includes nested control/MIDI interrupt work. Passing a static loop budget is not a measured CPU-utilization guarantee. |
| Static RAM | Recorded MVP build: 89,932 / 98,304 B, leaving 8,372 B. Existing ambient FM body field adds 96 B across the allocated 4×6 FM states. Do not extrapolate from only eight currently sounding voices when allocating state. |
| Buffer pool | Recorded: 330,208 / 344,064 B, leaving 13,856 B. Build policy reserves at least 8,192 B, leaving only **5,664 B** for extra pool buffers under that policy. |
| Main FX buffers | Delay 131,072 B; chorus 4,096 B; shared room/spring comb plus allpass storage 11,868 B. Duplicating the reverb buffers for stereo exceeds the remaining discretionary pool space. Longer delay lines and a second complete FX chain need a deliberate memory tradeoff. |
| Code/flash | Recorded image 441,300 B; linker XIP region 0x8DFBC = 581,564 B. Other flash regions store loader/data; image space is not free sample capacity. Keep package and recovery layout fixed. |
| GRAIN workload | 12 grains/part, capped aggregate decode load of eight source samples per output sample; reverse decoding counts twice. Incremental ADPCM index building and source changes create additional bursts. More Density can hit the cap rather than make an arbitrarily denser cloud. |
| CPU checks | Host regression allows +25% versus its instruction baseline; target loop-cost heuristic allows +10%. Recorded audio ISR cost 38,515 versus baseline 37,502 (+2.7%). These are regression checks, not 2.7% total CPU use or 7.3% spare processing time. |

The exact live worst-case load is **not established by this assessment**.
Stress four-note Tide with Texture, dense Cloud/reverse grains, Pulse release
overlap, high sends, rapid knob changes, MIDI/USB activity and screen redraws.
Also test duplicated expensive sounds across all four interchangeable slots.
Record max/typical half-buffer time, late/nested counters, shedding and note
loss. The onboard small speaker should not be the sole judge of low-end body;
compare line/headphone output with identical speaker-processing settings.

Likely cost centres are FM operator sample loops, granular decode/start/index
bursts, filters and always-running shared effects, plus controller work nested
in audio. Additional active FM operators cost more even when their state was
already allocated. Modulating `P_E*` through the matrix can repeatedly run
`fm6_sync()` and reset its patch LFO; prefer the existing live index path for
continuous motion. No cache-invalidation rewrite is needed for the first test.

Sources: [core.h](../firmware/src/core.h), [dsp.c](../firmware/src/dsp.c),
[voice.c](../firmware/src/voice.c), [audio.c](../firmware/src/audio.c),
[app.ld](../firmware/app.ld), [build.py](../tools/build.py),
[target_budget.txt](../tests/target_budget.txt),
[cpu_baseline.txt](../tests/cpu_baseline.txt).

## 4. Reference lessons and architecture decision

The Ambient Ø Rev.3 manual describes two assignable LFOs per layer (printed
pp.34–36), multi-oscillator structures (pp.44–46), nine reverb variations with
shimmer (p.37), and structure-dependent voice consumption (pp.28–29). Its
strength is coordinated timbre and motion with a declared resource tradeoff;
its feature count is not a specification for AmbWave.
[Sonicware manual](https://www.sonicware.co.jp/DL/LIVEN_Ambient/DOC/Ambient0_manual_en_r3.pdf).

AMB-1 is useful evidence that a modest source can feel complete through
independent breathing voices, timbral dynamics, filtering and placement. Its
documented overall sound also includes echo/plate processing. A plate port
should be separately costed; another firmware running on the FM-1 does not
establish that its effects fit alongside Felucca's retained buffers and engines.
[AMB-1 repository and credits](https://github.com/charlesvestal/fm1-amb).

For the specific prior audition, the locally retained source at revision
`d9ef0a35823bbad8edc7cc0feb2eae59539e4081` was inspected directly:
`pad_breathe`, `pad_coefs`, `render_pad`, `pad_bus`. The Rhodes renderer uses
one FM carrier plus a small second harmonic, **not a detuned carrier stack**.
Four voices have different breathing timing, amplitude/index coupling and pan;
the active pad bus has a two-pole low-pass and no chorus. Older comments claim
two saws/chorus; the executed code differs. This makes automatic attribution
of its fullness to oscillator count particularly weak.

### Option A versus Option B

| Choice | Audible potential | Cost and consistency | Decision |
| --- | --- | --- | --- |
| A: one flexible main Tide architecture | FM6 already spans grounded, harmonic and lush recipes. Allows deliberate fundamental/body balance and continuous evolution. | Reuses voice allocation, ramps, patch data and tests. Easier to tune a consistent macro response. | **Choose for the next milestone.** |
| B: multiple structures behind the same macros | Different sonic identities can be valuable; Warm/Analog and Silk/FM already partly implement this. | Each family needs its own control tuning, levels, tail/mode tests and CPU stress cases. Switching engines is a faded sound change, not a continuous morph. | Retain existing colours; add no new structure until the main dry sound passes. |

Longer term, a few curated structures selected with the existing preset
gesture are compatible with the philosophy. Do not make Shape jump between
engines or expose a new synthesis-mode menu. A useful first range is one
convincing source with meaningful macros, not three unfinished architectures.

## 5. Prioritized implementation plan

Effort is relative: S = confined recipe/control work; M = DSP plus integration;
L = shared routing or persistence work. Impact is a hypothesis to test by ear.

| Order | Concrete work | Expected audible impact | Effort / risk | Gate |
| --- | --- | --- | --- | --- |
| **P0a** | Isolate current FM Tide; audition bounded independent per-note breathing, as specified below. | High potential for organic depth; spectral body may still need work. | S–M / low–medium | Clear dry preference at matched level; no common pitch drift or extra voices. |
| **P0b**, only if needed | Tune the same FM recipe's carrier balance, ratios and envelopes. If static body is still insufficient, audition one quiet additional fundamental pair using idle operators, with very small fixed detune and gain compensation. | High potential for fullness. | S–M / medium: more active operators, cancellation, limiter pressure | Test static recipe separately from breathing; reject mere loudness/brightness advantage. Keep existing Warm/Silk as comparisons. |
| **P1a** | Trial mono per-track HPF after distortion/slicer, before level, pan and **all sends**. Smooth coefficients; support exact bypass. | High for four-layer clarity; no claim of improving solo Tide. | M / low–medium | Texture/Cloud lose mud without hollowing the intended body; test low notes and wet tails. |
| **P1b** | Extend existing four-fader interaction with a selected parameter row. Start Volume and High-pass only; audition a broad Low tone later if needed. | Medium–high usability and mix coherence. | M / medium, including storage | Labels show actual assigned sounds. Defaults follow sound identity, not physical slot. Navigation must preserve speed/length/mute gestures. |
| **P2** | After P0's listening result, separate timbre motion from slower body/breath changes; bounded random target timing and optional longer phrase-scale evolution. | Medium–high over minutes. | M / medium | Five-minute listening at several Motion settings; Motion0 still, no synchronous pumping or pitch wobble. |
| **P3a** | Measure ROOM impulse/decay and sustained-chord tails; audition send low cut/damping and diffusion adjustments using the current shared buffer budget. | Medium–high for depth and separation. | M / medium | Stable tails, no persistent DC/limit cycles, controlled low end. Compare with unchanged dry source. |
| **After P0a: stereo assessment** (formerly P3b) | Cost an inexpensive decorrelated L/R output from the existing network before considering a replacement tank; evaluate chorus placement separately. Implement only after assessing the routing and mono fold-down tradeoffs. | High potential for width, limited by existing network quality. | S assessment; M–L implementation / medium–high | Mono fold-down, L/R balance, impulse decorrelation and sustained listening; retain memory reserve. No promise that two taps equal a stereo reverb. |
| Library work alongside accepted DSP | Curate 2–3 Tide recipes and a few complementary four-layer combinations. Try loopable, licensed Cloud sources via existing sample slots. | High, relatively inexpensive. | S–M / low technically | Level-balanced auditions, source-loop quality, attribution and upload/fallback tests. |
| **Performance save/restore**, ahead of morphing | Define and reliably restore complete ambient sound identities, macro values, levels/mutes, modes and performance notes as specified by the recall design. | High product value: preserves a coherent atmosphere. | M–L / medium–high | Versioned compatibility, exact round trips, restart, safe note ownership and interrupted-save tests. |
| **Later: morphing** | Build 15/30/60 s transitions on accepted performance persistence. | Valuable performance evolution, low immediate dry-tone impact. | L / high | Safe interpolation of levels/timbre/density; discrete engines/samples need separate transition semantics and voice budgets. |

The HPF trial can start with bypass/40/80/120/180/250 Hz choices, with 6 dB/oct
first and a two-stage 12 dB/oct option only if needed. Listening starting
points, **not accepted factory defaults**: Tide bypass or 30–40 Hz; Texture
100–180 Hz; Cloud 70–140 Hz; Pulse bypass or 40–80 Hz depending on register.
Prefer sound-aware defaults, since all slots can hold any sound. Reuse the
master low-cut arithmetic/error-feedback pattern, not its shared state or
fixed coefficient. A later Low tone should be a broad shelf; defer Mid tone
until spectra/listening identify a repeatable problem. Do not use a common
master HPF or FM's index control as a substitute for a track mix filter.

Proposed dedicated Mix-page layout, using current boot order:

| Selected row | Knob 1: Tide | Knob 2: Texture | Knob 3: Pulse | Knob 4: Cloud |
| --- | --- | --- | --- | --- |
| Volume | Track 1 level | Track 2 level | Track 3 level | Track 4 level |
| High-pass | Track 1 cutoff | Track 2 cutoff | Track 3 cutoff | Track 4 cutoff |
| Low tone, if justified later | Broad shelf | Broad shelf | Broad shelf | Broad shelf |

Headers must follow actual slot assignments. SELECT could choose rows only
while the dedicated Mix page is focused; the existing HOME-held overlay stays
on volume and retains its speed/length/mode gestures. Prototype this with the
current four-card UI before settling navigation. Do not reorder stored tracks
to match the conceptual Tide/Texture/Cloud/Pulse table, and defer a Mid tone
row until it offers a demonstrated advantage over these simpler controls.

## 6. Focused first experiment: breathing FM body

**Question:** is independent note-level amplitude/index motion the missing
ingredient between the accepted AMB-1 dry reference and current Tide?
This experiment intentionally leaves static oscillator architecture unchanged,
so its answer is interpretable. It must earn the next default by listening.

1. Freeze current `828ceaa` as A. B uses the same patch, three active operators,
   gains, four-note cap, note events, pitch and Density mapping. No chorus,
   reverb, delay, pan spread, new filter or extra carrier in this comparison.
2. Give each active Tide note a separately phased smooth breath at control
   rate, with small deterministic timing variations. Start around 12–25 s
   per note; audition bounded amplitude movement of roughly 0 to -2 dB,
   never full silence. Couple a restrained FM-index bloom to each breath.
   These are starting ranges, not a prescribed optimum.
3. Motion scales breath depth and activity; Motion0 bypasses the new behavior
   exactly. Shape still sets complexity; Density still sets harmonic weight;
   Space still controls perspective and Space0 is dry. Avoid duplicating the
   same full-depth FM drift on top of the breath; compare a restrained blend.
4. Reuse `fm6_render`'s amplitude ramp and live `m.cutoff` index path, plus the
   existing easing/RNG techniques. Keep state outside serialized patches.
   Reset it correctly on note reuse, stealing, sound/mode changes and panic.
   Budget at most 1 KiB added static RAM as an experiment ceiling, verified
   from the target map; expected state should be much smaller. Do not alter
   the sample loops, voice count or allocate new delay buffers for this test.
5. Render one low note, the existing Dm7 `[50,57,65,72]`, and a higher/spaced
   chord; use identical velocity/timing/seeds. Supply raw and fixed-gain
   matched dry A/B, 30–60 s plus a five-minute sustained take, and release
   tails. Match one declared sustained-window RMS within 0.2 dB; never
   normalize individual breaths. Record peak/DC, spectral body balance,
   pre-limiter peaks/gain reduction and post-release silence. Repeat a
   practical test at unchanged mixer gain to expose headroom costs.
6. Ask the listener whether B has more body, warmth and evolving depth at
   matched loudness, without sounding like tremolo or a thinner pad. Include
   Motion0/25/50/100 and held-note knob gestures. Keep dry channels identical
   during this test so stereo placement cannot explain the preference.

**Pass:** a repeatable listener preference in low and middle registers,
continued interest over minutes, usable four-knob ranges, clean release,
unchanged voice cap, acceptable target timing and all regression gates.
Numeric spectral change alone is not success.
For the declared hardware stress cases, require no late audio halves or
overload shedding and worst observed total half-buffer work below the existing
85% shedding threshold; record the observation duration and MIDI/UI load.
This is a measured test envelope, not a guarantee for all possible patches.

**If it fails:** retain A and record the result. Test P0b's static carrier
voicing next, independently. Do not add effects to make a failed dry-source
experiment appear successful. These two small experiments separate dynamic
richness from missing static body before any new engine proposal.

## 7. Reuse and compatibility boundaries

Local comparison with upstream ref `7414269` shows ANALOG, PHASE, GRAIN,
NOISE and the modulation matrix unchanged in the relevant engine files.
FM6/core have a small ambient-only live harmonic-gain addition; voice.c adds
smooth DRIFT, and fx.c adds ambient level smoothing/dry trim. Build on these
components instead of importing another firmware's entire DSP/control stack.

- Reuse the common allocator, envelopes where applicable, engine-switch fades,
  parameter descriptions, preset gesture, existing render harnesses and native/
  WebAssembly comparison. FM6 amplitude/index paths can animate held notes;
  its patch operator envelopes can shape note onset/release.
- Reuse the mono track buffer for a small mix filter. A per-track filter needs
  only a few persistent states; the important work is smoothing, bypass,
  fixed-point range, state reset, UI and persistence semantics.
- Keep sample zone lookup, ADPCM decoding, user upload validation and missing-
  source fallback. User-loadable material is already technically possible;
  simpler discovery/curation is a separate UX issue.
- Preserve append-only engine IDs and parameter IDs. Never reinterpret
  unrelated stored parameters to make room for HPF/tone controls.

FUN8 projects currently store a 3,584-byte record including packed FM6 patches;
old formats import. They preserve engine parameters and patch data, but
`project_restore_runtime()` deliberately clears ambient sound identity.
Ambient macro state, latches, playing-mode metadata and live `carrier_body`
are not a complete serialized scene. Thus a saved FM patch loading correctly
does **not** prove an identical ambient performance is restored, especially
when Density depends on runtime state.

For P0 keep record sizes and loader behavior unchanged, preserve old projects'
generic sound-loading behavior, and keep the candidate opt-in until accepted.
Do not infer ambient identity from any arbitrary FM patch. If exact new macro
recall is required, specify a versioned, optional ambient-state extension with
old-file defaults and corruption/round-trip tests as separate work before
promising that compatibility. Persistent HPF/tone and future scenes need the
same explicit design; a runtime-only filter trial must be labelled unsaved.

## 8. Validation evidence and remaining work

The unchanged upstream build/test prerequisite is recorded in
[AMBIENT_PLAN.md](../AMBIENT_PLAN.md), milestone 0 (2026-10-07). It was not
rerun for this documentation-only investigation. Before future product code,
use a topic branch based on `ambient-prototype` and retain that baseline.

During this assessment, `tools/build-ambwave-macos.sh` reached `./build.sh`
but stopped with **Docker is not running**. No fresh target binary or memory
measurement was produced. Memory/image/ISR figures above are the recorded MVP
build evidence, not newly measured headroom.

The fresh host run through `tools/test-macos.sh` (which runs
`sh tests/run_tests.sh`) completed with **ALL HOST TESTS PASSED**:

- 92 golden audio renders, zero changed/gone; no health, voice/routing or
  CPU-budget failures. No golden or budget files were updated.
- Ambient-specific tests including dry default/dense FM Tide, zero spatial
  returns, held-note Density response and clean release. The default Dm7
  render measured RMS 2,898.7, peak 9,314, DC 0.0 and no remaining voices;
  these establish signal health, not richness.
- 184,320 ambient HOME/MIX layouts, zero lint/alignment findings; storage,
  preset, project, input, MIDI/USB, web and installer simulations passed.
- Standard browser emulator built and matched 88,200 native-rendered frames
  bit for bit. This default emulator build does not replace an explicit
  `AMBIENT_POC=1` browser comparison for a future sound candidate.
- Optional DaisySP reference comparison skipped because its checkout is
  absent; the official V15 restore case skipped because its package is absent.

The run used existing generated firmware headers/package and target
disassembly. Host source checks are fresh; passing package/target checks
against those existing artifacts cannot certify a newly built target image.
The session logs are `/tmp/ambwave-assessment-build.log` and
`/tmp/ambwave-assessment-tests.log` (temporary local evidence).

For each future candidate, require:

- Fresh AmbWave target build, linker memory/reserve checks and unchanged
  target-cost thresholds; runtime timing on FM-1 before release.
- Dry routing and held-note response, macro endpoints/corners, low/high pitch,
  release silence, no DC/clipping, allocator/stealing and all-slot stress.
- Full host suite: unchanged generic golden renders, storage versions,
  presets, UI layout/input, MIDI/USB and native/WebAssembly agreement.
- New long-form audio comparisons tied to the current public source revision,
  fixed seeds and declared matching. Some older audition scripts reference
  historical commits absent from this clean source snapshot; repair their
  baseline selection before relying on them for reproducibility.
- Listening acceptance distinct from numerical regression success. Update an
  affected golden hash only after the intentional sound change is reviewed;
  preserve every unaffected baseline and threshold.

No hardware installation is part of this assessment or implicitly authorized
by producing a package.

### Listening evidence and causal limits

- [Dry-source experiment](TIDE_DRY_SOURCE_RESULTS.md): less harsh, not
  necessarily richer.
- [Harmonic exchange experiment](TIDE_HARMONICS_RESULTS.md): different,
  not clearly richer.
- [AMB-1 reference](AMB1_DRY_REFERENCE.md): both dry references judged fuller,
  Rhodes more grounded. References include voice breathing, filtering and
  stereo placement, and differ in register/recipe; they do not isolate FM
  versus formants or detune as the cause.
- [FM patch audition/integration](TIDE_FM_PATCH_AUDITION.md): got closer;
  default source preserved, full macro-range listening acceptance pending
  in that record. [Release smoke checks](../CHANGELOG.md) and a successful
  hardware install are not proof of the stronger richness criterion.
- [Warm/Silk macro results](TIDE_MACROS_RESULTS.md): demonstrate existing
  alternate architectures and held-note macro tests; also document mono wet
  routing. Earlier architecture/prototype documents describe historical
  analog Tide and should not override current source.
