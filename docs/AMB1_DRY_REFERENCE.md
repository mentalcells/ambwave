# AMB-1 dry reference audition

2026-10-15. The user supplied Tyler.l's 49-second video
[AMB-1 Firmware!](https://www.youtube.com/watch?v=7qTpiEUz5EQ) after finding both
Tide source experiments different but not sufficiently richer.

These recordings are generated from AMB-1 source, not the video's audio. They
are not a reconstruction of its selected piece, settings or performance. The
agent has not heard the video or these recordings. The purpose is to let the
user identify whether AMB-1's intrinsic dry pad character is the desired target.

## Listening result

The user found **both dry reference pads richer and fuller**. The first (Rhodes)
was described as having more low-end grounding. Use Rhodes as the primary body
reference, with Voices as a second example of the desired fullness. This accepts
the reference direction, not either of the two rejected Tide candidates.

The Rhodes and Voices recipes have different keys/registers, four breathing
voices and stereo pan placement. Therefore, the next Tide source comparison
must use identical notes, voice count, register and loudness before attributing
the difference to FM or formant synthesis alone.

## Source and isolation

- Repository: https://github.com/charlesvestal/fm1-amb
- Pinned revision: `d9ef0a35823bbad8edc7cc0feb2eae59539e4081`.
- Unmodified `firmware/src/dsp/amb.c`, `amb.h`, `fastmath.h`, GPL-3.0-only.
- Local copies, audition harness and outputs: `build/amb1_reference/`.
- Built natively with `cc -O2 -ffp-contract=off`; no firmware package is built
  or installed from this third-party project.

The [manual](https://github.com/charlesvestal/fm1-amb/blob/d9ef0a35823bbad8edc7cc0feb2eae59539e4081/docs/MANUAL.md)
describes four independently breathing pad voices and a substantial echo/plate
reverb contribution to the overall sound. The current source has two pad types:

- **Rhodes:** same-frequency FM carrier/modulator. Its modulation index blooms
  with each breath, with a small second harmonic. The active render uses one
  carrier per pad voice; it does not blend a second detuned carrier.
- **Voices:** a triangle/saw/sine source through two vowel formants, retaining
  low-pass energy for body. Vowel position evolves slowly per voice.

Both pass through the existing pad bus low-pass. The active bus has **no chorus**,
although older header/inline comments still mention one. See the actual
`pad_bus()` and `render_pad()` implementations rather than those stale comments.
Four independent breath cycles, pan positions, slow pitch/pan/vowel movement and
the source's dynamics remain in these reference recordings; this is an isolated
pad audition, not a detuning/modulation-free scientific oscillator comparison.

## Recordings

The harness uses the Garden recipe for Rhodes and the Voices recipe for Voices,
muting bells, bass and pulse before the first rendered frame. Drift is set to
its longest interval to keep the initial harmony throughout each 40-second take.
Each process starts with clean engine/effect buffers and the same fixed seed.

For `*-dry.wav`, Echo and Reverb are both zero before play. The harness verifies
their actual send gains are zero and both echo delay buffers remain identically
zero after rendering. The active pad path has no chorus. `*-space.wav` retains
the recipe's echo/reverb settings with the same isolated pad and performance.
Source render and mixer inspection confirms the reverb has no input in the dry
case and starts from cleared memory.

Each `*-matched.wav` applies one fixed gain, matching stereo RMS over seconds
8–28 to 6000/32768. No EQ, compressor, added spatial effect or moving gain is
used. Normalized peaks stay below clipping. The recipes/harmonies differ between
the two colours; these are source auditions, not identical-note Tide A/B renders.

Files:

- `rhodes-dry-matched.wav`, `rhodes-space-matched.wav`.
- `voices-dry-matched.wav`, `voices-space-matched.wav`.
- `levels.json`: raw RMS, applied gain and peak bounds.
- `render.c`: harness; run one take per process so engine state starts clean.

The next decision is by listening: which dry colour, if either, has the missing
body/expressiveness? Then compare its dry/effected takes to understand how much
of the reference appeal remains with the spatial returns removed. This does not
authorize importing AMB-1 DSP, changing Hasake's controls, or installing firmware.
