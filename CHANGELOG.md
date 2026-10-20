# Changelog

## Unreleased

- Independent role and compatible sound selection on all four tracks.
- Autonomous DRONE role using the existing analog soft pad, chord replacement,
  slow evolution and sustained playback independent of PLAY/STOP.
- Existing Felucca factory pad, choir, bell, marimba and electric-piano auditions.
- TIDE BOWED METAL and BOWL STUDY resonances, with brightness, drift, excitation
  and reverb controls using the existing PHYS engine.
- Firmware and full host suite pass, including unchanged golden engine renders,
  CPU/storage guards, exhaustive layouts and native/WebAssembly parity.

These changes are development source; published v0.1.0 assets and the hosted
browser demo are unchanged. Complete scene recall and ORDERED arp playback are
not implemented. Hardware musical acceptance and measured timing remain pending.

## 0.1.0 — 2026-10-17

First AmbWave MVP, previously developed as Hasake, based on Felucca 1.0.5.2.
First public AmbWave release.

- Four independent tracks with TIDE, TEXTURE, CLOUD and curated pluck sounds;
  TIDE WARM and TIDE SILK remain selectable variations.
- SHAPE, MOTION, DENSITY and SPACE, direct track mixing and mutes.
- User-reviewed default layer balance makes PULSE audible over a sustained TIDE
  chord while retaining the accepted TIDE patch and macro defaults.
- HOLD, SCATTER and SEQUENCE playing modes, with the existing step editor.
- TIDE now uses the user-selected FM patch. Its dry source has sustained FM
  motion; Shape changes complexity and Density changes harmonic weight without
  chorus or detuning. Space 0 has zero spatial sends.
- Browser demo uses the same synthesis code; no hardware needed.
- Visible browser level sliders and mutes, with first-use playing guidance.
- Prepared direct browser installation with exact-package checksum checking,
  backup acknowledgement, progress, restart checks and previous-package rollback.
- Public maintainer identity: mentalcells; clean source history with upstream
  and third-party attribution retained.
- AmbWave branding and separate browser save namespace. Earlier Felucca/Hasake
  browser saves are left untouched and are not automatically imported.

Limits: eight shared voices, shared effects buses, mono effect return, possible
voice stealing under dense layering, and no complete curated-scene save/recall.
The browser does not emulate device USB, TRS MIDI or user sample slots. Device
USB/package/storage identifiers and recovery tools remain Felucca-compatible.

Hardware validation reported by the maintainer: browser installation, AMBWAVE
startup, basic playback and note release, SPACE, layer controls, and normal
power-cycle restart passed. Rollback and interrupted-update recovery remain
untested on real hardware. Software installer simulations cover those paths
but are not a substitute for device testing.
