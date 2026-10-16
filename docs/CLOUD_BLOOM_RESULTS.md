# CLOUD bloom experiment

2026-10-15, branch `codex/cloud-bloom`, based on `0e47fbc`.
The user enjoyed the installed playing-mode firmware, including CLOUD SCATTER,
and requested the proposed improvements with a way back.

## Controls

CLOUD HOLD accepts up to three latched pitches on any track. Tap a pitch again
to remove it; a fourth pitch is ignored until a place is freed. TEXTURE retains
its single register. The existing shared eight-voice allocation still applies.

For clocked CLOUD, HOME + ENV cycles SHORT / FLOW / LONG. FLOW is the default:
it gives grains more attack and release time. LONG extends them further. SHORT
uses the previous clocked attack/release and gate values. HOLD stays sustained
and the length gesture explains that without changing it.

Untouched CLOUD SCATTER starts at half notes. HOME + SELECT still sets speed,
and explicit speed choices survive length, mode and sound changes. Length is
independent of grain DENSITY. Sequence notes and rests are retained by length
changes. A long envelope does not guarantee unlimited overlap: all tracks share
eight voices, so full sustained chords and long clocked releases can steal.

No engine DSP, storage format, bootloader or installation protocol changes.
TIDE tuning is unchanged. The new package has not been installed on hardware.

## Validation

`tools/build-hasake-macos.sh` succeeded: image 439280 bytes, static RAM
89836/98304, pool 330208/344064. Target audio interrupt estimate 38515 against
the unchanged baseline 37502 (+2.7%, within the existing 10% limit).

Focused playing tests cover the three-note limit and toggle behavior on all
tracks, TEXTURE's retained single register, the physical HOME + ENV gesture,
speed preservation, the original SHORT parameters, preserved sequence data,
and sustained HOLD. The existing four-track sequence/voice-pressure test passes.
All three CLOUD length profiles pass dry/wet renders for audible output,
headroom and quiet tails after STOP. HOME/MIX covers 184320 layouts with zero
lint or alignment findings.
The explicit Hasake WebAssembly emulator passes its integration tests and
matches native performance renders bit for bit. Emulator bundle:
`build/emu/bloom/`, served at `http://127.0.0.1:8790/bloom/`.

Full `tools/test-macos.sh` completed with ALL HOST TESTS PASSED: 92 unchanged
golden renders, no audio health/voice-routing failures, no CPU entries over
budget, and passing persistence, UI, installer and browser checks. No golden
hashes or CPU thresholds were changed.

## Rollback

Tag `codex/playing-modes-before-cloud-bloom` preserves the old firmware source
and exact package installed on the box. The package and its digest are recorded
in `artifacts/rollback/playing-modes-4c42c71/README.md`. Checking out the tag
does not install anything; restoring hardware requires an explicit flash request.

## Next sound work

Prioritize TIDE patch tuning before more engines or features. Compare the same
chord and register at matched loudness: current TIDE, a warm saw pad with less
beating/chorus and gentle drive, and a softer triangle pad. Compare dry first,
then with identical shared effects. Evaluate onset, sustained body and release.
Use the user's M-VAVE demo as a reference when a link is available; no match
has yet been established. Choose the strongest patch by listening before
changing the default and remapping SHAPE/MOTION/DENSITY around it.
