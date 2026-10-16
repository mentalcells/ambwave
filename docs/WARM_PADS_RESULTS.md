# Warm pad comparison

This records the first comparison build. The subsequent user-requested macro
expansion and TIDE variation naming are in `docs/TIDE_MACROS_RESULTS.md`.

2026-10-15, branch `codex/warm-pads`, based on CLOUD bloom `c624217`.
The user requested soft, full, rich and warm sounds, with NatLife's Ambient &
Dreams video as a reference. The video's audio was not accessible to the agent;
these are original patch candidates, not a claimed reproduction of that bank.

## Audition

The original startup and original TIDE remain available. HOME's PRESETS list is
now TIDE, TEXTURE, CLOUD, WARM PLUCK, SOFT MALLET, BELL PLUCK, WARM PAD, SILK PAD.
Every track can select either new pad, retaining its playing mode, pattern,
clock speed, level and mute. As before, changing sound releases local input and
resets its sound macros. Both pads are TIDE-family variations; HOME's footer
names the specific variation, as it does for the PULSE timbres.

From startup TIDE, PRESETS two steps backwards selects WARM PAD. One step
forward selects SILK PAD, then another returns to original TIDE. Each sound
change clears the chord, so tap the same notes again for a comparison. Start
with a three-note chord; turn SPACE down for the dry sound. In the browser,
focus the PRESETS knob and use arrow keys, or drag/scroll it. HOLD still allows
up to four pitches, and HOME + ARP selects HOLD / SCATTER / SEQUENCE.

The new emulator is `build/emu/pads/`, at `http://127.0.0.1:8790/pads/`.
The older `bloom/` and `modes/` bundles remain separate. A local server must be
running (`python3 -m http.server 8790 --bind 127.0.0.1 --directory build/emu`).

## Sounds and macros

WARM PAD uses the existing ANALOG saw pair: resonance 6, drive 12, no oscillator
noise, stronger filter keyboard tracking, and a gentler envelope than original
TIDE. SHAPE moves cutoff 48..88. DENSITY increases oscillator blend, detune
0..16 cents and chorus 0..40; at the midpoint detune is 7 cents and chorus 16,
compared with original TIDE's 16 cents and chorus 52. MOTION adds restrained,
slow filter drift with no pitch modulation.

SILK PAD starts from Felucca's own FM6 PAD factory patch, algorithm 5's three
carrier/modulator pairs. It has an even carrier sustain, a softer carrier onset,
slower modulator onset, and no patch pitch LFO depth. SHAPE adjusts the existing
modulator-level macro -24..4; DENSITY adds carrier spread 0..32 and chorus 0..40;
MOTION adds gentle modulator-level drift. FM6 owns its operator envelopes, so
mode changes set those envelopes rather than relying on the ineffective track
ADSR. Clocked modes use faster onsets and releases than HOLD.

Both pads boot their local SPACE amount at 35: reverb send 42, delay send 0.
SPACE can increase either later. The shared bus settings and the other tracks
are unchanged. No engine DSP, new samples, storage formats or installer changes.

## Validation

The Hasake firmware build succeeds: image 440116 bytes, static RAM
89836/98304, pool 330208/344064. Dry/effected renders use identical C3/G3/E4
chords, velocities, track level and FX sends, with chorus disabled to isolate
the sources. Sustained dry RMS: original TIDE 7628, WARM PAD 7696, SILK PAD
5801; their loudness is not exactly equal. Peaks stay below 17209/32767,
DC stays small, and all three release to silence without hanging voices.
Generated reference WAV files are in `build/warm_pad_demo/`.

Focused tests cover four-note chords at every new-pad macro corner, all
tracks/modes, preset-wheel reachability, retained patterns/levels/global FX,
and mode-change release. Browser tests exercise physical three-note pad chords
and match native onset/sustain/release samples bit for bit. The HOME/MIX sweep
also exercises both new TIDE timbres in every palette/style/size.

The full `tools/test-macos.sh` suite reports ALL HOST TESTS PASSED: 92 unchanged
golden renders, no audio health or routing failures, no CPU entries over budget,
and 184320 HOME/MIX layouts with zero lint/alignment findings. The expanded
four-track macro/voice-pressure render passes with original TIDE and each new
pad. Target audio interrupt cost remains 38515 versus baseline 37502 (+2.7%,
within the unchanged 10% limit); FM6 render-loop cost remains at baseline.
No hashes or budget thresholds were raised. The emulator was opened, switched
on and visually checked with both pad names; WARM PAD was left selected.

## Comparison and rollback

Original TIDE's mappings and startup are unchanged. `c624217` is the committed
pre-pad comparison point and its `bloom/` emulator is retained. The earlier
installed package remains preserved under the tag
`codex/playing-modes-before-cloud-bloom` and in
`artifacts/rollback/playing-modes-4c42c71/`. The new package has not been flashed.
Musical quality and on-device CPU margin still require hardware listening.
