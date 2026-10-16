// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
// The browser build (build/emu/felucca.wasm, web/emu/felucca_web.c) in Node, as the worklet runs it. After X0X's
// tests/host/emu_test.mjs (charlesvestal/fm1-x0x, GPL-3.0).
//   node web/emu/emu_test.mjs build/emu/felucca.wasm [NATIVE.f32]
// Checks: it boots (splash, then HOME on the screen, the HOME LED), a held key sounds and lights its LED, PLAY
// starts the sequencer (PLAY's green LED), a user preset saved from the panel (SAVE, OCT+, OCT+) reaches the flash,
// and the flash sectors kept as the page keeps them bring it back in a fresh instance; the same gestures twice
// give the same audio (deterministic), with NATIVE.f32 (web/emu/native_check.c) the native build's, bit for bit.
// Then the cost: 1 s of a heavy song (FM6, PHYS, GRAIN chords, DRUM) against real time (BENCH_MAX, default 0.5:
// fails above half of real time on this machine).
import fs from "fs";
import { storeSectors, restoreSectors } from "./worklet.js";

const wasmPath = process.argv[2] || "build/emu/felucca.wasm", nativePath = process.argv[3];
const bytes = fs.readFileSync(wasmPath);
const B = { FX: 0, SCL: 1, ENV: 2, LFO: 3, EDIT: 4, GLO: 5, HOME: 6, SAVE: 7, ARP: 8, SEQ: 9, PLAY: 10, REC: 11, OCTDN: 12, OCTUP: 13 };
const EN = { SELECT: 0, ALGO: 1, PRESETS: 2, K1: 3, K2: 4, K3: 5, K4: 6 };
let fails = 0;
const check = (what, ok) => { console.log(`emu: ${what.padEnd(76)} ${ok ? "ok" : "FAIL"}`); if (!ok) fails++; };

async function device(saved) {
  const { instance } = await WebAssembly.instantiate(bytes, {});
  const ex = instance.exports, mem = ex.memory;
  ex._initialize();
  ex.web_nor_erase();
  if (saved) restoreSectors(new Uint8Array(mem.buffer, ex.web_nor(), ex.web_nor_size()), saved);
  ex.web_boot();
  const d = { ex, mem, peak: 0, sig: 0, rec: false, frames: 0, kept: [] };
  d.render = (ms) => {
    const n = Math.round(ms * 44.1);
    for (let k = 0; k < n; k += 128) {
      const m = Math.min(128, n - k);
      ex.web_render(m);
      const l = new Float32Array(mem.buffer, ex.web_out_l(), m), r = new Float32Array(mem.buffer, ex.web_out_r(), m);
      for (let i = 0; i < m; i++) {
        d.peak = Math.max(d.peak, Math.abs(l[i]), Math.abs(r[i]));
        if (d.rec) {
          d.sig = (d.sig * 31 + Math.round(l[i] * 32768) + 7 * Math.round(r[i] * 32768)) % 1000000007;
          d.kept.push(l[i], r[i]);
        }
      }
      d.frames += m;
    }
  };
  d.tap = (b) => { ex.web_buttons(1 << b); d.render(60); ex.web_buttons(0); d.render(60); };
  d.turn = (role, n) => { for (let i = 0; i < Math.abs(n); i++) { ex.web_enc(role, Math.sign(n)); d.render(40); } };
  Object.defineProperty(d, "samples", { get: () => new Float32Array(d.kept) });
  d.screen = () => new Uint16Array(mem.buffer, ex.web_screen(), 240 * 240).slice();
  return d;
}
const colours = (fb) => new Set(fb).size;

// ---- boot, keys, screen, LEDs
const a = await device(null);
a.render(200);
const splash = colours(a.screen());
a.render(800);
const home = a.screen();
check(`boots: the splash, then HOME (${colours(home)} colours), the HOME LED lit`,
      splash > 1 && colours(home) > 4 && (a.ex.web_lit_buttons() >> B.HOME & 1) === 1);
const ambient = a.ex.web_ambient_poc() === 1;
if (ambient) {
  const mixer = await device(null); mixer.render(1000);
  mixer.ex.web_keys(1 << 7); mixer.render(100); mixer.ex.web_keys(0); mixer.render(100);
  const signature = mixer.ex.web_ambient_param_signature(), held = mixer.ex.web_ambient_latch_count();
  const writes = mixer.ex.web_flash_writes_count();
  mixer.ex.web_mix_level(2, 120); mixer.ex.web_mix_mute(0, 1); mixer.render(200);
  check("browser mixer changes a background level/mute without changing held notes or TIDE macros",
    (mixer.ex.web_mix_state(2) & 255) === 120 && !!(mixer.ex.web_mix_state(0) & 256) &&
    mixer.ex.web_ambient_param_signature() === signature && mixer.ex.web_ambient_latch_count() === held);
  mixer.ex.web_mix_level(2, 999); mixer.ex.web_mix_level(99, 0); mixer.ex.web_mix_mute(99, 1);
  check("browser mixer bounds tracks/levels and does not write saved flash",
    (mixer.ex.web_mix_state(2) & 255) === 127 && mixer.ex.web_mix_state(99) === 0 && mixer.ex.web_flash_writes_count() === writes);
  mixer.ex.web_mix_mute(0, 0); mixer.render(100);
  check("browser unmute retains the held TIDE chord", !(mixer.ex.web_mix_state(0) & 256) && mixer.ex.web_ambient_latch_count() === held);
}
if (ambient)
  check("AMBIENT_POC boots into the curated scene with empty sequencers and stopped transport",
        a.ex.web_test_ambient_boot() === 1);
check("silent at rest", a.peak === 0);
a.ex.web_keys(1 << 7);                                        // C4
a.render(300);
const keyLit = (a.ex.web_lit_keys() >> 7) & 1;
a.ex.web_keys(0);
a.render(400);
check(`a key sounds (peak ${a.peak.toFixed(3)}) and lights its LED`, a.peak > 0.01 && keyLit === 1);
if (ambient) {
  check("AMBIENT_POC boots on the focused TIDE performance HOME",
        a.ex.web_ambient_home_active() === 1);
  check("AMBIENT_POC DENSITY boots at original TIDE midpoint",
        a.ex.web_ambient_density() === 50 && a.ex.web_ambient_density_values() === 512);
  a.turn(EN.K3, -128);
  check("AMBIENT_POC KNOB 3 reaches thin DENSITY",
        a.ex.web_ambient_density() === 0 && a.ex.web_ambient_density_values() === 0);
  a.turn(EN.K3, 128);
  check("AMBIENT_POC KNOB 3 reaches broad DENSITY",
        a.ex.web_ambient_density() === 100 && a.ex.web_ambient_density_values() === 1024);
  check("AMBIENT_POC SHAPE boots at its curated default",
        a.ex.web_ambient_cutoff() === 0 && a.ex.web_ambient_shape() === 50);
  a.turn(EN.K1, -128);
  check("AMBIENT_POC KNOB 1 reaches the dark SHAPE bound",
        a.ex.web_ambient_cutoff() === -16 && a.ex.web_ambient_shape() === 0);
  a.turn(EN.K1, 1);
  check("AMBIENT_POC SHAPE rises monotonically from the dark bound",
        a.ex.web_ambient_cutoff() === -16 && a.ex.web_ambient_shape() === 1);
  a.turn(EN.K1, 128);
  check("AMBIENT_POC KNOB 1 reaches the bright SHAPE bound",
        a.ex.web_ambient_cutoff() === 12 && a.ex.web_ambient_shape() === 100);
  check("AMBIENT_POC MOTION boots as subtle smooth filter drift",
        a.ex.web_ambient_motion() === 25 && a.ex.web_ambient_motion_values() === 0x00050a09);
  a.turn(EN.K2, -128);
  check("AMBIENT_POC KNOB 2 reaches still MOTION without pitch wobble",
        a.ex.web_ambient_motion() === 0 && a.ex.web_ambient_motion_values() === 0x00050006);
  a.turn(EN.K2, 128);
  check("AMBIENT_POC KNOB 2 reaches the safe animated MOTION bound",
        a.ex.web_ambient_motion() === 100 && a.ex.web_ambient_motion_values() === 0x0005181e);
  check("AMBIENT_POC SPACE boots at the original TIDE tuning with room above it",
        a.ex.web_ambient_space() === 50 && a.ex.web_ambient_space_values() === 0x6c52522a &&
        a.ex.web_ambient_space_detail() === 0x7fff4c36);
  a.turn(EN.K4, -128);
  check("AMBIENT_POC KNOB 4 reaches the close SPACE bound",
        a.ex.web_ambient_space() === 0 && a.ex.web_ambient_space_values() === 0x6c520000 &&
        a.ex.web_ambient_space_detail() === 0x7fff4c36);
  a.turn(EN.K4, 128);
  check("AMBIENT_POC KNOB 4 reaches the safe wide SPACE bound",
        a.ex.web_ambient_space() === 100 && a.ex.web_ambient_space_values() === 0x6c527f6e &&
        a.ex.web_ambient_space_detail() === 0x47e04c36);
  check("AMBIENT_POC key-up leaves C4 latched and its key LED lit",
        a.ex.web_ambient_latch_count() === 1 && ((a.ex.web_lit_keys() >> 7) & 1) === 1);
  a.ex.web_keys(1 << 7); a.render(80); a.ex.web_keys(0); a.render(80);
  check("AMBIENT_POC pressing C4 again removes it and its key light",
        a.ex.web_ambient_latch_count() === 0 && ((a.ex.web_lit_keys() >> 7) & 1) === 0);
  for (const key of [7, 9, 11, 12, 14]) {
    a.ex.web_keys(1 << key); a.render(80); a.ex.web_keys(0); a.render(80);
  }
  const latchLights = [7, 9, 11, 12].every(key => ((a.ex.web_lit_keys() >> key) & 1) === 1) &&
                      ((a.ex.web_lit_keys() >> 14) & 1) === 0;
  check("AMBIENT_POC latch keeps four-note headroom and shows those four keys",
        a.ex.web_ambient_latch_count() === 4 && latchLights);
  a.ex.web_buttons(1 << B.FX); a.render(80);
  a.ex.web_buttons((1 << B.FX) | (1 << B.OCTDN)); a.render(80);
  a.ex.web_buttons(1 << B.FX); a.render(80);
  a.ex.web_buttons(0); a.render(160);
  check("AMBIENT_POC FX + OCT- clears the latch", a.ex.web_ambient_latch_count() === 0);
}
if (ambient) {
  const layers = await device(null); layers.render(1000);
  layers.ex.web_keys(1 << 7); layers.render(60); layers.ex.web_keys(0); layers.render(60);
  layers.turn(EN.ALGO, 1);
  layers.ex.web_keys(1 << 7); layers.render(60); layers.ex.web_keys(0); layers.render(60);
  check("TEXTURE focus/hold preserves TIDE independently", layers.ex.web_ambient_latch_count() === 1 && (layers.ex.web_texture_state() & 5) === 5);
  layers.ex.web_buttons(1 << B.HOME); layers.render(800);
  const before = layers.ex.web_texture_state(); layers.turn(EN.K2, 1);
  check("HOME hold opens MIX and adjusts TEXTURE level", (layers.ex.web_texture_state() & 2) === 2 && (layers.ex.web_texture_state() >> 16) === (before >> 16) + 1);
  layers.ex.web_buttons(0); layers.render(60);
  check("HOME release closes momentary MIX", (layers.ex.web_texture_state() & 2) === 0);
  layers.ex.web_buttons(1 << B.FX); layers.render(60); layers.ex.web_buttons((1 << B.FX) | (1 << B.OCTDN)); layers.render(60);
  layers.ex.web_buttons(1 << B.FX); layers.render(60); layers.ex.web_buttons(0); layers.render(60);
  check("TEXTURE clear leaves TIDE sounding", (layers.ex.web_texture_state() & 1) === 0 && layers.ex.web_ambient_latch_count() === 1);
  layers.turn(EN.ALGO, 1);
  layers.ex.web_keys(1 << 7); layers.render(60); layers.ex.web_keys(0); layers.render(60);
  check("PULSE input queues silently while stopped", (layers.ex.web_pulse_state() & 0x3ff) === 1);
  layers.tap(B.PLAY); layers.render(60);
  check("PLAY runs PULSE and preserves TIDE", (layers.ex.web_pulse_state() & 0x200) !== 0 && layers.ex.web_ambient_latch_count() === 1);
  const pulseBefore = layers.ex.web_pulse_state();
  layers.turn(EN.PRESETS, 1);
  check("PULSE sound selection retains HOME, level and transport, and clears old input", layers.ex.web_pulse_timbre() === 1 &&
    (layers.ex.web_pulse_state() & 0xff) === 0 &&
    (layers.ex.web_pulse_state() & 0xffff0000) === (pulseBefore & 0xffff0000) &&
    (layers.ex.web_pulse_state() & 0x200) !== 0);
  layers.ex.web_keys(1 << 7); layers.render(60); layers.ex.web_keys(0); layers.render(60);
  layers.turn(EN.ALGO, -1); layers.turn(EN.ALGO, 1);
  check("PULSE remains selected/reachable after changing its sound", layers.ex.web_pulse_timbre() === 1 && layers.ex.web_ambient_home_active() === 1);
  const controlsBefore = layers.ex.web_pulse_controls();
  layers.ex.web_buttons(1 << B.HOME); layers.render(60); layers.turn(EN.SELECT, 1);
  check("HOME SELECT changes selected PULSE speed without changing density", (layers.ex.web_pulse_controls() & 3) === 3 &&
    (layers.ex.web_pulse_controls() & 0xff00) === (controlsBefore & 0xff00));
  layers.ex.web_keys(1 << 5); layers.render(60); layers.ex.web_keys(0); layers.render(60);
  layers.ex.web_buttons(0); layers.render(60);
  check("HOME black key latches PULSE mute and preserves its input", (layers.ex.web_pulse_controls() >> 16) === 4 &&
    (layers.ex.web_pulse_state() & 0xff) === 1);
  layers.turn(EN.K3, 1);
  check("DENSITY changes probability without changing speed", (layers.ex.web_pulse_controls() & 3) === 3 &&
    (layers.ex.web_pulse_controls() & 0xff00) > (controlsBefore & 0xff00));
  layers.ex.web_buttons(1 << B.HOME); layers.render(60);
  layers.ex.web_keys(1 << 5); layers.render(60); layers.ex.web_keys(0); layers.render(60);
  layers.ex.web_buttons(0); layers.render(60);
  check("second HOME black-key tap restores PULSE", (layers.ex.web_pulse_controls() >> 16) === 0);
  layers.tap(B.PLAY); layers.render(60);
  check("STOP releases PULSE but retains inputs", (layers.ex.web_pulse_state() & 0x3ff) === 1);
  const slots = await device(null); slots.render(1000);
  check("fourth layer boots as CLOUD", (slots.ex.web_ambient_layer_state(3) & 0xff) === 4);
  for (let k = 0; k < 4; k++) {
    if (k) slots.turn(EN.ALGO, 1);
    for (let n = 0; n < 8; n++) {
      slots.turn(EN.PRESETS, 1);
      check(`slot ${k + 1} HOME survives sound selection ${n + 1}`, slots.ex.web_ambient_home_active() === 1);
    }
  }
  slots.ex.web_keys(1 << 7); slots.render(60); slots.ex.web_keys(0); slots.render(60);
  check("fourth-layer CLOUD key remains latched", ((slots.ex.web_ambient_layer_state(3) >> 8) & 0xff) === 1);
  slots.ex.web_buttons(1 << B.HOME); slots.render(60);
  slots.ex.web_keys(1 << 8); slots.render(60); slots.ex.web_keys(0); slots.render(60);
  slots.ex.web_buttons(0); slots.render(60);
  check("fourth HOME black key mutes CLOUD and retains its input", (slots.ex.web_pulse_controls() >> 16) === 8 &&
    ((slots.ex.web_ambient_layer_state(3) >> 8) & 0xff) === 1);
  const modes = await device(null); modes.render(1000); modes.turn(EN.ALGO,3);
  modes.tap(B.SEQ);
  check("CLOUD SEQ gesture selects sequence mode", modes.ex.web_ambient_mode(3) === 2);
  const phraseKeys = [7,11,14,9,7,11,14,9], phraseNotes = [60,64,67,62,60,64,67,62];
  for (const key of phraseKeys) { modes.ex.web_keys(1 << key); modes.render(60); modes.ex.web_keys(0); modes.render(60); }
  check("CLOUD STEP editor writes all eight physical notes", phraseNotes.every((note,i)=>modes.ex.web_ambient_step_note(3,i)===note));
  const phraseSignature = modes.ex.web_ambient_pattern_signature(3);
  modes.tap(B.HOME); modes.turn(EN.PRESETS,1);
  check("changing CLOUD to a pluck retains SEQUENCE and its phrase", modes.ex.web_ambient_mode(3) === 2 &&
    modes.ex.web_ambient_pattern_signature(3) === phraseSignature && modes.ex.web_ambient_home_active() === 1);
  modes.turn(EN.PRESETS,-1); modes.peak=0; modes.tap(B.PLAY); modes.render(2000);
  check("written CLOUD phrase produces audio", modes.peak>0.01);
  modes.tap(B.PLAY);
  modes.ex.web_buttons(1 << B.HOME); modes.render(60);
  modes.ex.web_buttons((1 << B.HOME)|(1 << B.ARP)); modes.render(60);
  modes.ex.web_buttons(1 << B.HOME); modes.render(60); modes.ex.web_buttons(0); modes.render(60);
  check("HOME ARP cycles SEQUENCE back to HOLD without losing the phrase", modes.ex.web_ambient_mode(3)===0 &&
    modes.ex.web_ambient_pattern_signature(3)===phraseSignature && modes.ex.web_ambient_home_active()===1);
  if (process.argv[4]) {
    const comparison = await device(null); comparison.render(1000); comparison.rec = true;
    for (let stage = 0; stage < 5; stage++) { comparison.ex.web_test_texture_stage(stage); comparison.render(1000); }
    const nat = new Float32Array(new Uint8Array(fs.readFileSync(process.argv[4])).buffer), web = comparison.samples;
    check("four layers, per-hit variation, speed, mute/resume and STOP/restart = native bit for bit",
      nat.length === web.length && nat.every((sample, i) => sample === web[i]));
  }
  if (process.argv[5]) {
    const pads = await device(null);pads.render(1000);pads.rec=true;
    for(let pad=0;pad<2;pad++) {
      pads.turn(EN.PRESETS,-1);pads.peak=0;
      pads.ex.web_keys((1<<7)|(1<<11)|(1<<14));pads.render(60);pads.ex.web_keys(0);pads.render(4000);
      check(`warm pad ${2-pad} retains HOME and a sounding three-note chord`,pads.ex.web_ambient_home_active()===1&&((pads.ex.web_ambient_layer_state(0)>>8)&255)===3&&pads.peak>0.01);
      pads.ex.web_keys((1<<7)|(1<<11)|(1<<14));pads.render(60);pads.ex.web_keys(0);pads.render(4000);
    }
    const nat = new Float32Array(new Uint8Array(fs.readFileSync(process.argv[5])).buffer), web = pads.samples;
    check("SILK/WARM pad chords, onset and release = native bit for bit",nat.length===web.length&&nat.every((sample,i)=>sample===web[i]));
  }
}
a.render(1500);
a.peak = 0;
a.ex.web_midi(0x90, 64, 100);                                  // Web MIDI: E4 on channel 1
a.render(300);
a.ex.web_midi(0x80, 64, 0);
a.render(300);
check(`a MIDI note in (USB-MIDI's queue) sounds (peak ${a.peak.toFixed(3)})`, a.peak > 0.01);
const draws = a.ex.web_screen_draws();
a.turn(EN.K1, 3);
check("KNOB 1 redraws the screen", a.ex.web_screen_draws() > draws);
a.tap(B.PLAY);
a.render(1000);
check("PLAY: the sequencer remains available and its green LED lights", a.ex.web_playing() === 1 &&
      (a.ex.web_lit_buttons() >> 14 & 1) === 1);
a.tap(B.PLAY);
a.render(300);
check("PLAY again stops it", a.ex.web_playing() === 0);
a.ex.web_buttons(1 << B.FX);                                  // #119: FX held, its map: the effects breathe
a.render(800);
const brK = a.ex.web_breath_keys(), brB = a.ex.web_breath_buttons(), litK = a.ex.web_lit_keys();
a.ex.web_buttons(0);
a.render(300);
check("FX held: its button and the effects' keys breathe, none lit; let go: nothing",
      (brB >> B.FX & 1) === 1 && brK !== 0 && (brK & litK) === 0 && litK === 0 &&
      a.ex.web_breath_keys() === 0 && a.ex.web_breath_buttons() === 0);
{ // the page's breathing peaks against the firmware's (hal/fm1_input.h FM1_LED_BREATH_PK / _LO, /256 of lit): the
  // colour at 50 % between the dark and the lit one by the LED's share, gamma-encoded (sRGB), within 0.06
  const html = fs.readFileSync(new URL("./index.html", import.meta.url), "utf8");
  const hal = fs.readFileSync(new URL("../../firmware/hal/fm1_input.h", import.meta.url), "utf8");
  const pk = (n) => +hal.match(new RegExp(`#define ${n} (\\d+)u`))[1] / 256;
  const srgb = (x) => x <= 0.0031308 ? 12.92 * x : 1.055 * x ** (1 / 2.4) - 0.055;
  const red = (c) => parseInt(c.slice(1, 3), 16);
  const share = (name, lit) => {                        // (the red channel: the peak's share of dark .. lit)
    const m = html.match(new RegExp(`@keyframes ${name} \\{ 0%, 100% \\{ fill: (#[0-9a-f]{6}); \\} 50% \\{ fill: (#[0-9a-f]{6})`));
    return (red(m[2]) - red(m[1])) / (lit - red(m[1]));
  };
  const hi = srgb(pk("FM1_LED_BREATH_PK")), lo = srgb(pk("FM1_LED_BREATH_PK_LO")), litBtn = 0xff, litSlit = 0xec;
  const got = [share("breath-btn", litBtn), share("breath-slit", litSlit), share("breath-btn-lo", litBtn), share("breath-slit-lo", litSlit)];
  check(`the page's breath peaks ${got.map((x) => x.toFixed(2)).join(" ")} ~ ${hi.toFixed(2)} (HI) ${lo.toFixed(2)} (LO)`,
        Math.abs(got[0] - hi) <= 0.06 && Math.abs(got[1] - hi) <= 0.06 && Math.abs(got[2] - lo) <= 0.06 &&
        Math.abs(got[3] - lo) <= 0.06);
}

// ---- a user preset saved from the panel, kept across a fresh instance
const w0 = a.ex.web_flash_writes_count();
a.tap(B.SAVE);
a.tap(B.OCTUP);
a.tap(B.OCTUP);
a.render(200);
const saved = storeSectors(new Uint8Array(a.mem.buffer, a.ex.web_nor(), a.ex.web_nor_size()));
check(`SAVE, OCT+, OCT+: user preset 1 written (${Object.keys(saved).length} sectors kept)`,
      a.ex.web_test_user_preset(0) === 1 && a.ex.web_flash_writes_count() > w0 && Object.keys(saved).length > 0);
const kept = JSON.parse(JSON.stringify(Object.fromEntries(     // as the page keeps them (base64 in localStorage)
  Object.entries(saved).map(([k, v]) => [k, Buffer.from(v).toString("base64")]))));
const b = await device(Object.fromEntries(Object.entries(kept).map(([k, v]) => [k, new Uint8Array(Buffer.from(v, "base64"))])));
b.render(600);
const c = await device(null);
c.render(600);
check("a fresh instance with the kept sectors has it; one without does not",
      b.ex.web_test_user_preset(0) === 1 && c.ex.web_test_user_preset(0) === 0);

// ---- deterministic: the same gestures, the same samples
async function song() {
  const d = await device(null);
  d.render(500);
  d.ex.web_test_heavy();
  d.tap(B.PLAY);
  d.rec = true;
  d.render(2000);
  return d;
}
const s1 = await song(), s2 = await song();
check(`the heavy song plays (peak ${s1.peak.toFixed(3)}), twice the same samples`, s1.peak > 0.05 && s1.sig === s2.sig);
if (nativePath) {                                             // web/emu/native_check.c: the same song, built with cc
  const nat = new Float32Array(new Uint8Array(fs.readFileSync(nativePath)).buffer);
  const web = s1.samples;
  let diff = nat.length === web.length ? 0 : Infinity;
  for (let i = 0; i < web.length && diff === 0; i++) if (nat[i] !== web[i]) diff = i + 1;
  check(`the same song from the native build (cc): ${web.length / 2} frames, ${diff === 0 ? "bit for bit" : "differs at " + diff}`, diff === 0);
}

// ---- the cost: 1 s of the heavy song against real time (the best of 3, after 2 s to settle)
let best = Infinity;
s1.rec = false;
for (let i = 0; i < 3; i++) {
  const t0 = performance.now();
  s1.render(1000);
  best = Math.min(best, performance.now() - t0);
}
const ratio = best / 1000, max = +(process.env.BENCH_MAX || 0.5);
check(`1 s of the heavy song (FM6 PHYS GRAIN chords + DRUM) in ${best.toFixed(1)} ms: ${(ratio * 100).toFixed(1)} % of real time`, ratio < max);
console.log(fails ? `emu: ${fails} FAILED` : "emu: all ok");
process.exit(fails ? 1 : 0);
