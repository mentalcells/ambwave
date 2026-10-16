# Two-layer TEXTURE prototype

Implementation on `codex/ambient-texture`, based on accepted DENSITY `f383c1b`
and the specification committed as `22b4054`. The user subsequently installed
the package manually and gave positive feedback on TIDE + TEXTURE and MIX.
See `HASAKE_DESIGN_DECISIONS.md` for the accepted direction and remaining checks.

## Playing

1. Boot into **1 TIDE**; toggle up to four keyboard notes into a held chord.
2. Turn ALGORITHM right to **2 TEXTURE**, then press a key to start the noise bed.
   Key release keeps it running. The same mapped pitch toggles it off; another
   pitch changes its register. Scale and octave still determine the mapped pitch.
3. The four knobs edit the selected layer's SHAPE, MOTION, DENSITY and SPACE.
4. Hold HOME for **MIX**: knob 1 TIDE level, knob 2 TEXTURE level. Knobs 3/4
   are inactive, and PRESETS turns are ignored while mixing. Release HOME to
   restore macros. Levels use Felucca's existing
   nonlinear level curve; displayed percentages describe control position.
5. FX + OCT- clears the selected layer. PLAY still controls the transport and
   never creates a pattern or clears the two held layers.
6. EDIT opens the full interface. HOME hold there still opens the menu; HOME
   tap returns home. Full UI retains all four tracks and existing save/load paths.

HOME shows the selected layer/sound prominently, both levels, TIDE held count
and TEXTURE ON/OFF. Both layers boot silent; only tracks 3/4 are muted. Loading
another sound or project exits the curated macro/latch policy for affected
tracks; it clears their holds through the existing panic path and shows the
ordinary Felucca interface; the other layer's overview uses a generic TRACK1/2
label for the replaced sound. Curated macro state is runtime-only, restored at
boot. This milestone does not serialize a scene or reinstate macros on undo.

## Sound and ownership

TIDE's accepted sound and default macro parameters are unchanged. TEXTURE uses
NOISE ANLG: COLR 76, FREQ 63, RES 18, tracking 24, intrinsic DRFT/CRSH 0;
ADSR 88/96/120/96, LEVEL 80, no chorus. Its macro defaults are 35/25/25/50.
SHAPE moves COLR 100 -> 30 and FREQ 42 -> 102; MOTION uses the same bounded
smooth track DRIFT policy as TIDE; DENSITY moves the existing impulse control
0 -> 58 (roughly 0 -> 32 impulses/sec) over a continuous bed. No loudness
compensation was needed. This is intentionally sparse; hardware listening
must assess whether its crackle is distinct enough against a TIDE chord.

TEXTURE SPACE changes its sends and dry gain only. TIDE retains sole ownership
of shared delay/reverb bus settings. Selecting layers does not reapply effects.
Consequently TEXTURE's apparent space changes when TIDE changes the shared
buses; there are no independent reverbs. Dry gain changes remain layer-local.

A register change reuses the sounding NOISE voice with the existing
`voice_start` envelope/filter preservation when neither pitch belongs to an
external MIDI source. With external ownership it instead releases/reallocates
through the normal allocator, preserving that source's old pitch. Outgoing
physical note-ons are balanced on toggles, register changes, clear and loads.
External note-off does not release a physical hold.

MIX slews level changes over blocks and interpolates within each block, using
Felucca's block-ramp approach. The default gain is exact at boot. Existing
mute, engine transition and voice stealing paths remain in use.

## Validation

- `tools/build-hasake-macos.sh`: hardware package built with `AMBIENT_POC=1`;
  unchanged loader/package identity. App 435552 bytes, data+bss 89772/98304,
  pool 330208/344064. The upstream remote remains the official Felucca repo.
- Full required host suite and target budget checks passed (`ALL HOST TESTS
  PASSED`); the final repeat also covers overlapping HOME/FX gestures.
  Golden audio hashes and existing CPU budget thresholds were not changed.
  The optional PHYS/DaisySP reference check was skipped because DaisySP is not
  installed; PHYS stability tests still passed.
- New tests cover both holds/focus/clears, outgoing MIDI, independent incoming
  ownership, MIX press/hold/release and macro isolation, full UI/menu access,
  PLAY, sound loading, reset and repeated release-tail pressure.
- HOME/MIX layout lint covers every theme, FLAT/LINE and normal/large cards:
  zero overlap, clipping, ink/neutrality or alignment findings. MIX also retains
  display priority when an FX quick-layer map is open.
- All 16 TEXTURE macro corners with both TIDE extremes, at default and maximum
  mixer levels: peak 21638 (<32767), worst DC 99.4, late tail RMS 142.6 at
  9 seconds after release at 4 seconds. No unbounded output or stalled tails.
  Host rendering cost was about 0.3% of realtime; this is not a measured FM-1
  cycle count. The target static audio-ISR cost is 40651 against budget 37502,
  inside the existing +10% allowance. The bounded once-per-key register search
  is a separate helper, like Felucca's keyboard latch helper, rather than
  inlined into sample loops; DSP-loop budget thresholds are unchanged.
- Default ten-second comparison WAVs (four seconds held, then release) in
  `build/texture_demo`: TIDE RMS ~7437, TEXTURE ~1514, combined ~7386.
  TEXTURE starts approximately 14 dB below the four-note TIDE chord.
- Isolated TIDE default render compared with an unchanged `f383c1b` firmware
  source snapshot: byte-identical SHA-256
  `c4d9c72caed3c9061bff535c61a838ebc1129cb5abe87849ec51903a0daa501d`.
  This comparison does not replace or update any golden audio hashes.
- Hasake browser tests cover layering, MIX and selected clear. The dedicated
  native/WebAssembly comparison includes both layers, a register change and
  independent releases: all 220500 frames matched bit for bit.

## Voice-pressure limitation and foundation decision

Forty repeated four-note chord replacements plus TEXTURE register changes
preserved all five held voices while releasing/stealing old tails within the
shared eight-voice budget. No allocation policy or global voice cap changed.

External MIDI has no reserved headroom: twelve simultaneous external pitches
on layer 2 filled the shared budget and left only one of four held TIDE voices
sounding. The physical hold ledger can therefore list a pitch whose voice was
stolen. MIDI note-offs and clear still completed without stuck gated voices.
Keep external MIDI within the shared budget during this prototype; reserving
physical voices would require a separately considered routing/allocator policy.

Software evidence supports hardware evaluation of the immediate two-layer
workflow. The sound balance, usefulness of sparse crackle, real hardware CPU
margin and shared-space interaction still need listening. Do not expand a
scene library until the user has evaluated this foundation.

## Hardware listening checklist (after explicit approval)

- Latch a four-note TIDE chord; select TEXTURE and start it without a menu.
- Move TEXTURE's register and sweep each macro, especially DENSITY.
- Hold HOME, fade each layer to zero and back, then release to resume macros.
- Clear TEXTURE while TIDE holds; restart it and clear TIDE independently.
- Repeat chord changes while both play; check for dropped notes/clicks.
- Try TIDE SPACE at 0/50/100 while TEXTURE stays wet; assess shared FX limits.
- Check PLAY/STOP leaves both holds intact; enter EDIT and verify menu access.

The earlier DENSITY install approval does not authorize flashing this package.

## Package

`build/hasake-texture.fwsc` (609849 bytes), SHA-256:
`865b00d700bfc891a8927b154ef7f6e56dc63ea2ee8db4fa36215a18ce404eb7`.

This evaluation package received positive user hardware feedback on layering
and mixing; the complete edge-case listening checklist is not yet recorded.
