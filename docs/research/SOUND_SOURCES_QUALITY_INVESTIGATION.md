# Sound sources and quality roadmap

2026-10-20. Investigation only: no firmware, presets, macros, effects, storage,
build outputs or hardware changed by this investigation. Existing working-tree
changes were left in place. No new listening verdict or hardware timing claim
is made here; audible benefits and incremental costs below are estimates.

## Recommendation

Use the engines and sounds we already have first. Add a few **unmodified
Felucca factory presets** to the compatible role lists, then audition a small
subset of the now-public **official V15 DX7 bank** through our existing FM6.
AMB-1 explains a useful target, but its pads/bells are custom renderers, and
its eight pieces are whole-instrument recipes rather than importable patches.
For space, investigate upstream Felucca's existing stereo HALL before porting
AMB-1's substantially larger plate/tape implementation.

Do not merge a new upstream firmware, change platform, add a generator, build
a new engine, or copy a whole reference instrument as part of this work.

## Evidence and versions

Fresh, shallow, read-only source checkouts were made under `/tmp/` in the
priority order requested. These pinned trees, rather than cached `main` pages,
are the basis of the assessment:

| Source | Inspected commit | Important version distinction |
| --- | --- | --- |
| [AMB-1](https://github.com/charlesvestal/fm1-amb/tree/188b0a42f20a547864e732ae5ea1339d0fbcf395) | `188b0a42f20a547864e732ae5ea1339d0fbcf395` | Current DSP is `app/dsp/amb.c`, MPL-2.0; older GPL trees used `firmware/src/dsp/amb.c`. |
| [Felucca](https://github.com/hugelton/Felucca/tree/5738617968a22b5d4c0d3470cfcc95559052f446) | `5738617968a22b5d4c0d3470cfcc95559052f446` | Reports v1.6.1; our source base is v1.0.5.2. Its engine submodule points to `c46e0086f69bdc26df1cd144dca9e683cc4531dd`. |
| [Felucca-Engines](https://github.com/hugelton/Felucca-Engines/tree/04ffe903eb87a752305d1e5b10da01b53e4ab004) | `04ffe903eb87a752305d1e5b10da01b53e4ab004` | Standalone HEAD is ahead of Felucca's pinned submodule and now includes six MIT engines, including PHASE. |
| [FM1 Open](https://github.com/m-vave/fm1_open/tree/4d266850f8a46bd5154a0d495c1f60317156c9f7) | `4d266850f8a46bd5154a0d495c1f60317156c9f7` | V14. Supersedes the V13 source used in the earlier local SDK assessment, particularly for the factory bank. |

Local comparison includes `firmware/src/ambient.c`, `eng_fm6.c`, `eng_phase.c`,
`eng_analog.c`, `eng_formant.c`, `eng_phys.c`, `mod.c`, `fx.c` and
`tools/gen_fm6_patches.py`, with the implemented role/patch separation.

## 1. AMB-1: what is actually making the sound

Primary source: [app/dsp/amb.c](https://github.com/charlesvestal/fm1-amb/blob/188b0a42f20a547864e732ae5ea1339d0fbcf395/app/dsp/amb.c),
[amb.h](https://github.com/charlesvestal/fm1-amb/blob/188b0a42f20a547864e732ae5ea1339d0fbcf395/app/dsp/amb.h).

### Pads

`render_pad()` has two source colours:

- **Rhodes:** a sine carrier phase-modulated by a same-frequency sine, plus
  `0.12 * sin(2*phase)`. Modulation index increases with the breath envelope.
  One active carrier phase supplies this path; it is not a detuned unison pad.
- **Voices:** triangle/saw/sine excitation through two moving formant filters,
  retaining low-pass energy for body. Vowel interpolation and small per-voice
  formant differences supply a recognizable vocal spectrum.

`pad_breathe()`, `pad_coefs()` and `breath()` give the four pad notes independent
swells, unequal cycle lengths (factors 1, 1.21, 1.43, 1.67), varying strengths,
small pitch motion and distinct pan positions. This is coordinated evolution
of the voices, not simply adding more oscillators.

`pad_bus()` then darkens the stereo sum through two cascaded one-pole low-pass
stages. **There is no active pad chorus in the inspected code.** Some header
and inline comments still describe two detuned saws/ensemble chorus; the active
renderer and `pad_bus()` supersede those comments.

Our Body Pad already translates the Rhodes source idea into FM6: one 1:1 pair
and a quiet second harmonic. It omits AMB-1's independent voice panning, full
breath cycles, pad-bus low-pass and stereo space. Warm is ANALOG; Silk is an
FM6 three-pair PAD adaptation. A wholesale FM replacement therefore has less
justification than exposing additional existing sounds.

Prior local listening already found AMB-1 dry pads fuller, with Rhodes more
grounded, but those reference takes differed in notes/register and included
breathing and pan. The integrated Body Pad got closer. P0a breathing was more
noticeable without fixing source richness, and breathing tuning is paused.
See `docs/AMB1_DRY_REFERENCE.md`, `TIDE_FM_PATCH_AUDITION.md`, and
`TIDE_P0A_RESULTS.md`. Do not repeat that experiment as if it were a new finding.

### Bells

`bell_voice()` and `render_bells()` use **two-operator FM**, not our current
PHASE pluck. The modulator/carrier ratios are 1, 2, 3, 3.5 and 4.77; amplitude
and modulation index decay independently. Lower notes ring longer; strike
smoothing, velocity-dependent index and loop-specific ratio/index/decay give
individual hits different material and articulation. Those are source-sound
qualities that can be approximated with existing FM6 patch parameters.

By comparison our three Pulse sounds are PHASE SAW / DSIN / resonant TRI
variations, sharing a macro-shaped pluck envelope. Motion randomizes tone,
amplitude and track pan; Density sets SCATTER probability. Bell Pluck is not
Felucca's FM6 GLASS BELL. FM6 already offers independent operator envelopes and
inharmonic ratios without importing AMB-1's bell renderer.

AMB-1's fixed pan per bell loop differs from our latest-hit track pan: preserving
a separate stereo position for each ringing voice would require changes to our
mono-per-track render/mix path. Do not quietly include that in a preset import.

### Tape echo and plate

`echoes()` combines a roughly 160 ms fed-back Echoplex with a quarter-rate
11.025 kHz tape loop, two fractional read heads and different L/R output gains.
Feedback is progressively low-passed and high-passed. Slow modulation moves
head position, stereo placement and feedback balance; interpolation smooths
reads. `render_chunk()` sends some echo output into the reverb, so repeats
become part of the spatial wash rather than sitting separately beside it.

`reverb()` is a stereo Dattorro plate: 30 ms predelay, four input diffusers,
two cross-fed tank halves, modulated allpasses, damped delays and seven output
taps per side. Diffusion, modulation and distinct stereo taps are plausible
contributors to a smoother and wider atmosphere. These are implementation
inferences, not a listening comparison against our firmware.

These effects are reusable source, but **not drop-in additions**:

| Inspected AMB-1 storage | Bytes, buffers only |
| --- | ---: |
| Plate: 34,706 float samples | 138,824 |
| Tape: 62,000 int16 samples | 124,000 |
| Echoplex: 7,100 int16 samples | 14,200 |
| Total | 277,024 |

Our existing chorus/delay/reverb buffers total about 147,036 bytes. The last
role-separation build used pool 330,208/344,064 and static RAM 89,948/98,304:
only 13,856 pool bytes and 8,356 static bytes remain. Replacing those buffers
with AMB-1's trio adds roughly 130 kB before other states; adding them alongside
is worse. Its global instrument/float/fastmath model also differs from our
fixed-point engine interface. Porting just the complete plate would need a
separate memory strategy even if no new UI were added.

### Arrangement is separate from sound

The four bell phrases of 13/17/23/29 beats, chord voice-leading, scale choice,
slow ARC, occasional octaves/rests and Euclidean drums influence the overall
music. `PIECES` contains eight whole-scene recipes selecting pad colour,
levels, harmony, generator and effect settings. These are not eight pad patches.
We can audition pad and bell colours with our HOLD/SCATTER/SEQUENCE; none of
those composition systems is required for this roadmap.

## 2. Felucca and Felucca-Engines: the easiest reuse

[Felucca's patch generator](https://github.com/hugelton/Felucca/blob/5738617968a22b5d4c0d3470cfcc95559052f446/tools/gen_fm6_patches.py)
contains exactly the same eight FM6 patch definitions as our local generator
(structured-data equality checked). Updating upstream will not magically add
more FM6 factory sounds. We already possess TINE EP, GLASS BELL, SOFT PAD,
WOOD BARS/MARIMBA, DRAWBARS/ORGAN and NYLON PICK/PLUCK; they are simply not
exposed as separate curated role choices.

Useful sounds already compiled into our firmware:

| Role to audition | Existing factory sound | Actual local source |
| --- | --- | --- |
| Tide | FM6 PAD in its original form; ANALOG SOFT PAD, PWM STR, STRINGS | `eng_fm6.c`, `tools/gen_fm6_patches.py`, `eng_analog.c` |
| Tide / Texture | FORMANT CHOIR AAH, WHISPER | `eng_formant.c` |
| Pulse | FM6 BELL, MARIMBA, PLUCK, TINE EP; ANALOG SINE KEY / PLUCK | `eng_fm6.c`, `eng_analog.c` |
| Pulse, later budget test | PHYS KALIMBA, MARIMBA, HARP / PLUCK | `eng_phys.c`, `phys_dsp.c`, `phys_symp.c` |
| Cloud / Texture | GRAIN CLOUD PAD / FROZEN / SHIMMER and existing noise presets | `eng_grain.c`, `eng_noise.c`; retain source/grain/noise requirements |

FM6 complexity comes from operator ratios, independent envelopes, unequal
carrier levels, feedback, velocity response and detune. ANALOG pads use oscillator
blend/PWM/filtering. FORMANT supplies vocal resonances. PHYS uses resonating
modes/strings; its per-voice cost and slot limits require particular care.
Our existing `mod.c` already routes LFO, envelopes, velocity, key, per-note
random values, modulation wheel, aftertouch and expression to pitch/timbre/gain
and track parameters. Slow DRIFT and patch-internal envelopes can provide
movement without importing a generator or new modulation framework.
None requires a new engine for AmbWave. Saved pattern and factory FX settings
are distinct from the dry source; do not attribute their contribution to the
oscillator alone or load their patterns when auditioning a sound.

[Felucca-Engines](https://github.com/hugelton/Felucca-Engines/tree/04ffe903eb87a752305d1e5b10da01b53e4ab004)
provides fixed-point C99 ANALOG, LOFI, TRIO, WHEEL, NOISE and PHASE. Current
ANALOG adds sync/sub options and current PHASE adds folding; existing controls
and host contracts have evolved. Useful reusable building blocks, but neither
an FM6 bank nor an effects library. No need to replace these engines to expose
the presets already present in our fork. Do not substitute standalone HEAD for
Felucca's pinned submodule without compatibility work.

### Significant upstream effect improvement

[Felucca `firmware/src/fx.c`](https://github.com/hugelton/Felucca/blob/5738617968a22b5d4c0d3470cfcc95559052f446/firmware/src/fx.c)
now includes **HALL**, absent in AmbWave: an eight-line, mutually coupled,
modulated stereo FDN at 22.05 kHz, with input diffusion, damping and low cut.
It reuses ROOM's comb/allpass buffers; its additional state/work area is about
708 bytes at CTL=32. This is much more compatible with our memory budget than
the full AMB-1 plate. It is a hall, not a Dattorro plate.

Our ROOM and SPRING feed the same wet samples to both output channels.
Upstream HALL supplies a side block and a small `rev_side_mix()` stage before
master processing. A selective backport needs those helpers, clearing/model
transition code, and the side-routing hook, not the rest of v1.6.1.
Upstream `tests/reverb_test.c` covers stereo, decay, silence, model changes and
cost, but those tests were inspected, not rerun here. Its +30% relative reverb
and <=2 percentage-point bus cost limits are **upstream test expectations**,
not measurements or guarantees for AmbWave's four-layer build.

## 3. Official FM1 Open: reuse patch data, not its platform

V14 includes the original V15 **128-voice factory DX7 bank** in
[app/fm_factory.inc](https://github.com/m-vave/fm1_open/blob/4d266850f8a46bd5154a0d495c1f60317156c9f7/app/fm_factory.inc).
I decoded its lossless stream in a temporary analysis script: 16,384 bytes,
SHA-256 `ecad0b8620f9d784d46c8da1e3c5de02c40214649ed8dec218026431046aad25`,
matching the source declaration. This confirms the bank data, not its sound.

Specific audition candidates, **zero-based indices**:

- Tide: SYN PAD 1 (3), STRING 1 (66), VOICE 1 (67), then their numbered siblings.
- Pulse: FROGBELL (100), CLANG BELL (112), KALIMBA (117), TUB BELLS (120).

Names are leads, not quality rankings. These voices are standard 128-byte VMEM
records; our `fm6_unpack()`, `fm6_set_patch()` and `fm6_adopt()` already accept
that representation. Some original values exceed DX7's normal parameter
ranges; both official `app/fm_library.c` and our `eng_fm6.c` sanitize before
rendering. Retain attribution and audit sanitized results. Standard patch
compatibility does **not** imply audio identity between the official renderer
and ours (velocity, envelopes, gain, filtering and FX can differ).

The official `app/fm_factory.c` expands the whole bank into a 16 kB RAM array.
Do **not** copy that runtime strategy into our nearly full static RAM. Decode
offline and embed only selected 128-byte records in flash, then use existing
per-track patch RAM. No new bank storage, sample writes or SDK linking needed.

Official DX7 voices include their own pitch/amplitude LFO and operator-envelope
settings (`web/synth/dx7lfo.c`, `dx7env.c`). The non-DX7 track LFO in
`web/engine/sound.c:lfo_render()` supplies amplitude/pan modulation and smoothing;
it explicitly bypasses DX7, which uses its FM modulation path. Our existing
modulation matrix covers similar purposes, so this is reference behavior rather
than a missing engine capability to port wholesale.

The sixteen synthetic FM demo recipes in `app/presets.c` are a separate source
from that original factory library; avoid conflating them.

Other available material: portable 303, 808, PSG and plucked-string sources in
`third_party/synth/`, the MSFA-derived browser FM core in `web/synth/`, and the
open browser mixer/modulation in `web/engine/sound.c`. These have potential
reuse, but our analogous synthesis capabilities already exist. Importing their
engines or global instrument would be more work than importing their patch data.

`web/engine/effects.c` is a low-cost, eighth-rate four-comb reverb plus delay,
with different comb-pair stereo returns and filtered/interpolated output. It
is not a hidden plate algorithm or evidence of a better native reverb. Native
DSP/platform services reside in the separately licensed `libfm1_engine.a`.
The public browser implementation is not proof of native implementation or
hardware cost. The native SDK is outside this sound-library milestone.

## 4. Licensing and compatibility

- **AMB-1:** current inspected files are MPL-2.0, with author notices. Older
  versions including our previous `d9ef0a3` reference remain GPL-3.0-only.
  Current-tree copying requires MPL source/notice compliance and the GPL
  combination route under MPL section 3.3; no separate Exhibit B incompatible
  marking was found on the inspected DSP files (the example printed inside
  the license text is not itself such a file marking). Keep copied DSP isolated,
  preserve notices and document the additionally distributed GPL terms.
  Alternatively use the exact earlier GPL revision if its implementation is
  sufficient. Do not silently label current upstream code GPL-only.
  [Pinned licensing](https://github.com/charlesvestal/fm1-amb/blob/188b0a42f20a547864e732ae5ea1339d0fbcf395/LICENSING.md),
  [Mozilla FAQ Q14](https://www.mozilla.org/en-US/MPL/2.0/FAQ/).
- **Felucca:** GPL-3.0-only, matching our source distribution. Its fixed-point
  engine repository is MIT; MSFA FM core Apache-2.0; physical models retain MIT;
  sample assets must be checked individually rather than assumed covered by
  code licensing. Retain upstream and component credits.
  [Felucca licensing](https://github.com/hugelton/Felucca/blob/5738617968a22b5d4c0d3470cfcc95559052f446/LICENSING.md),
  [engine license](https://github.com/hugelton/Felucca-Engines/blob/04ffe903eb87a752305d1e5b10da01b53e4ab004/LICENSE).
- **FM1 Open:** APP/open browser source GPLv3 with its SDK linking permission;
  MSFA and portable synth components retain their respective Apache/MIT terms.
  The original bank's own header and THIRD_PARTY explicitly state Apache-2.0
  provenance from bundled V15. Preserve that material and audit the selected
  records' provenance before public distribution. The proprietary native
  binary has separate FM1-only terms; APP's exception is not permission to
  combine arbitrary GPL code with that binary. We need none of it for VMEM reuse.
  [License](https://github.com/m-vave/fm1_open/blob/4d266850f8a46bd5154a0d495c1f60317156c9f7/LICENSE.md),
  [component and bank notices](https://github.com/m-vave/fm1_open/blob/4d266850f8a46bd5154a0d495c1f60317156c9f7/THIRD_PARTY.md).

## 5. Ranked improvements

Rank balances expected audible benefit, effort, and performance risk; these
are proposals pending listening and target measurements.

| Rank | Improvement | Expected audible benefit | Effort | Incremental performance/memory |
| --- | --- | --- | --- | --- |
| 1 | Expose a handful of existing Felucca presets under Tide/Pulse | High potential variety: true FM bell/mallet vs three PHASE variants; vocal/PWM pad vs three current pads | Low–medium | No new DSP implementation. Same-engine presets have small metadata cost; FM6/FORMANT/PHYS can cost more than the source they replace. PHYS last. |
| 2 | Audition selected official V15 FM voices through existing FM6 | High potential variety without designing new patches | Low–medium | 128 flash bytes per selected voice plus names/metadata; existing per-track patch RAM. CPU depends on algorithm, active operators and tails. |
| 3 | Selectively backport Felucca stereo HALL | High potential wet-space smoothness/width; no change to dry source | Medium | Reuses reverb buffers, roughly 0.7 kB extra state. More DSP and stereo accumulation; measure on target. |
| 4 | Borrow a bounded AMB-1 tape-echo treatment | Medium–high potential atmosphere for bells and sparse notes | Medium | Reuse existing delay buffer; extra fractional reads, stereo tap and feedback filters. Downsampled long tape/plex combination deferred. |

**Integration details:**

1. Extend the role/patch catalog and initialization in `firmware/src/ambient.c`;
   use existing `ui.c:apply_preset_to()` and each engine's factory index. Add only
   small patch-profile dispatch so existing factory values survive at the
   initial macro positions. Current Pulse's `P_E0..P_E4` writes mean PHASE
   wave/DCW/detune; the same indices mean different things in FM6/FORMANT.
   Likewise current Tide mode handling rewrites FM operator envelopes for its
   known Body/Silk recipes. Do not run those overrides on unrelated factory
   patches. Keep role timing/probability separate from patch timbre writes;
   keep factory patterns and global FX out of loading. `eng_fm6.c`,
   `eng_analog.c`, `eng_formant.c`, `eng_phys.c` provide the actual sound data.
   This completes patch integration within the accepted model, not a new
   architecture or macro redesign. Unchanged source at defaults is an explicit
   acceptance check; any envelope adaptation must be labeled as an adaptation.
2. Offline decode `app/fm_factory.inc`; verify its documented hash and selected
   records. Feed chosen records to existing FM6 unpack/set/adopt paths, with
   neutral FM offsets and original operator envelopes. Do not port
   `fm_factory.c`'s full-bank RAM allocation, `fm_library.c` flash layout, native
   SDK, or newer upstream bank storage.
3. Extract `rev_hall()`/`hall_*`, required constants/state, `rev_clear()` additions,
   model transition handling and `rev_side_mix()` from upstream `fx.c`. Use the
   existing shared sends and SIZE/DAMP. For the first listening experiment keep
   the alternative opt-in and do not change saved G_RTYPE meaning or golden
   hashes. Production model selection would separately require descriptor,
   compatibility and UI tests. No full upstream merge is necessary.
4. In our `fx.c:fx_buses()`, evaluate one extra read head with distinct stereo
   output and a bounded low-cut/low-pass feedback path, taking techniques from
   AMB-1 `tape_read()`/`echoes()`. Keep existing delay timing/SPACE semantics,
   preserve zero-FX dry sound, and test mono compatibility. Current delay
   already has low-passed feedback: that alone is not a new improvement.
   Do not import the global AMB-1 renderer, a second full buffer, independent
   head choreography or the whole float dependency stack. A full tape model
   and plate remain separate research options if this smaller step fails.

## 6. One proposed audible A/B experiment

**Question:** Does an existing FM bell preset make Pulse more useful and
atmospheric than its current Bell Pluck, without another engine implementation?

- **A:** current Pulse Bell Pluck (PHASE, current defaults).
- **B:** Felucca FM6 BELL / GLASS BELL, factory index 1, original VMEM and neutral
  engine offsets. Use the existing FM6 engine; do not design a new patch or
  import the AMB-1 bell generator. Verify B's source parameters against a plain
  factory load before comparing.
- Same fixed 8-note phrase, notes/register, velocity, BPM, trigger times, and
  gate on both. Use SEQUENCE to remove random arrangement differences. Disable
  per-hit random modulation in both; retain each patch's intrinsic envelopes
  and timbre. No extra macros are moved during comparison.
- First compare dry (all spatial sends zero and clean buffers); then run the
  same phrase with identical current delay/reverb sends and bus settings. Do
  not compare different effects or a new hall/tape algorithm in this experiment.
- Match perceived level using one constant output gain per take over a declared
  phrase/tail window; preserve attacks and envelopes, provide raw plus matched
  takes, check peaks/DC and sufficient silence after release. Avoid automatic
  loudness matching that boosts a dying tail or changes the preset itself.
- Browser: A/B selector replays the same phrase from a clean state. Judge tone,
  attack softness, sustain/decay, high-register harshness and usefulness under
  one unchanged quiet Tide chord. More metallic is not automatically better.
- If the dry comparison is promising, build one opt-in package with A/B patch
  choices. Run `./build.sh` through the AmbWave wrapper and the host suite,
  regression audio, storage/UI and target CPU checks. Compare native/WASM
  renders and voice/tail pressure with all four layers before hardware listening.
- Hardware is a later listening stage with explicit approval in that
  conversation: same patch choices and test phrase, headphones first, then
  the FM-1 speaker/mono. Measure worst-case target timing and reject a sound
  that depends on clipping or steals the sustained Tide chord at normal use.

If B is worthwhile, keep it as an additional Pulse sound; no need to replace A.
If not, audition the existing FM6 MARIMBA or official bank before designing a
new sound or reworking effects. This is a controlled listening experiment, not
a claim that FM is inherently better than PHASE.
