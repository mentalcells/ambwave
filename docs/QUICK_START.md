# AmbWave quick start

AmbWave is an ambient instrument for the M-VAVE FM-1. The browser version uses
the same synthesis code. No hardware or personal samples are needed to try it.

## First scene

1. In the browser, click **Switch on** to enable audio. Start at a modest volume.
2. Track 1 starts on **TIDE / BODY PAD**. PRESETS selects another Tide sound.
3. Tap three notes. In HOLD mode they remain sounding; tap each again to release.
4. Set SPACE to 0 to hear TIDE dry. Its defaults are SHAPE 50, MOTION 25,
   DENSITY 50 and SPACE 50. Explore the first three knobs during a sustained note.
   The warm variations start at
   SHAPE 50, MOTION 25, DENSITY 50 and SPACE 35. Return there for the starting tone.
5. Turn ALGORITHM to track 2, tap a note for TEXTURE, and balance it quietly:
   use the visible browser mixer. On hardware, hold HOME and turn knob 2;
   the four knobs mix tracks 1–4.
6. Track 3 starts as PULSE SCATTER. Tap a few pitches, press PLAY, then balance
   it with the browser mixer or HOME + knob 3. STOP pauses generated notes
   and keeps its pitch set.

The starting mixer balance is TIDE 64%, TEXTURE 62%, PULSE 74%, CLOUD 62%.
These are fader positions, not equal-loudness percentages. The default balance
was reviewed with a sustained TIDE chord and scattered PULSE notes.

Hold HOME and turn PRESETS to choose TIDE, TEXTURE, CLOUD, PULSE or DRONE for the
focused track. Release HOME and turn PRESETS to choose a compatible sound:
TIDE keeps BODY PAD / WARM PAD / SILK PAD, followed by FM SOFT PAD, AN SOFT PAD,
AN STRINGS, AN PWM STR and CHOIR AAH. PULSE keeps WARM PLUCK / SOFT MALLET /
BELL PLUCK, followed by GLASS BELL, FM MARIMBA and FM TINE EP.
TEXTURE offers NOISE and CLOUD offers GRAIN. FM means FM6; AN means ANALOG.

The new factory auditions preserve the original timbres rather than applying
Body/Warm/Silk or PHASE-pluck overrides. **FACT** on SHAPE/MOTION (and TIDE
DENSITY) means that control is deliberately unmapped for Phase 1. SPACE starts
at 50, restoring the factory chorus/delay/reverb sends; 0 removes all three.
PULSE DENSITY continues to control SCATTER probability without changing FM tone.
Factory FM patches keep their own modulation and operator envelopes, so a held
pluck may decay naturally. Original sounds retain their existing macro mappings.

Changing role or sound clears that track's live held pitches and resets its
macros; speed, pattern, level and mute are retained. Playing mode is retained
except DRONE, which always uses sustained HOLD.
Retap your chord after a change. These choices do not select a complete scene.

## Autonomous Drone

Hold HOME and turn PRESETS one step backwards from TIDE to choose DRONE on
any track. Its single sound is AN SOFT PAD. Press a note or overlapping chord
(up to four notes), then release all keys. The Drone sustains and evolves
without PLAY, including while you play another track.

The first press after all keys are released replaces the previous harmony;
overlapping presses build the new chord. MIDI follows the same rule. SHAPE
sets brightness, MOTION adds slow independent timbre/breathing/drift, DENSITY
sets beating, and SPACE controls existing effects. Try MOTION 0, 25 and 100.
HOME + ARP keeps Drone sustained. HOME + black track keys mute without clearing;
**FX + OCT− clears the Drone**, allowing normal release tails. STOP leaves it
running. See [Drone details and five-minute auditions](DRONE_ROLE_RESULTS.md).

## Browser knobs and keys

Click a knob and use your computer's up/down arrow keys to turn it one step,
or drag/scroll over it. There is no separate preset Up button on the FM-1.
ALGORITHM changes track; PRESETS changes sound; HOME + PRESETS changes role.
The role appears in the track list and the selected sound appears in the footer.

Click the on-screen piano or use the computer keys shown beneath the instrument.
Shift-click a panel button to keep it held; Shift-click again to release it.
For example, Shift-click HOME to expose mixing, then release HOME to return
the four knobs to their sound macros. Only the focused browser tab receives keys.

## Four macros

- **SHAPE:** rounded/muted to open and harmonically rich.
- **MOTION:** still to stronger, irregular tonal movement. Allow a few seconds
  to hear slow settings. TIDE does not add pitch wobble.
- **DENSITY:** harmonic weight on TIDE, without detuning or chorus; the WARM
  and SILK variations retain their spread controls. For CLOUD it
  changes grains; for PULSE SCATTER it changes how often notes occur.
- **SPACE:** dry/close to a wash of delay and reverb. The effects buses are
  shared; this knob controls local sends and dry balance. The existing effect
  return is mono, so this is not independent stereo reverb on every track.

## Hold, scatter and sequence

**HOME + ARP** cycles HOLD → SCATTER → SEQUENCE. A mode change clears live input
but retains the stored pattern. **HOME + SELECT** sets SCATTER or SEQUENCE speed.

- HOLD: toggle sustained notes. TIDE and plucks hold up to four, CLOUD up to
  three, TEXTURE one register. All tracks share eight voices.
- SCATTER: toggle a pitch set and press PLAY for random notes from that set.
- SEQUENCE: press SEQ from HOME to open the existing step editor. Tap/release
  successive notes to enter a phrase. EDIT clears the current step and advances.
  Press SEQ again for pattern length/division/gate; HOME returns to the macros.

For clocked CLOUD, HOME + ENV cycles SHORT / FLOW / LONG note articulation.
This does not change grain DENSITY. HOLD remains sustained.

## Clear and mute

FX + OCT− clears held input on the selected track. HOME plus the first four
black keys toggles track mutes while retaining the notes. STOP releases generated
notes; held drones remain intentional. To clear a written sequence, open SEQ,
hold REC, then confirm with OCT+ (OCT− cancels).

## Current limits

This is an MVP. Complete scene save/recall is not implemented. Dense
chords and release tails can steal voices from other tracks. The older installed
firmware is preserved for rollback; opening the browser never flashes a device.
