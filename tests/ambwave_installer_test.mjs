// SPDX-License-Identifier: GPL-3.0-only
// Exercise the real installer page logic with the actual package and a fake
// updater. No browser MIDI/device access is requested by these tests.
import fs from "node:fs";
import vm from "node:vm";
import assert from "node:assert/strict";
import { webcrypto } from "node:crypto";
import { productOf, logicalImage } from "../web/fm1pkg.js";

const bytes = new Uint8Array(fs.readFileSync("build/felucca.fwsc"));
const hash = Buffer.from(await webcrypto.subtle.digest("SHA-256", bytes)).toString("hex");
const html = fs.readFileSync("web/ambwave_install.html", "utf8");
const script = html.match(/<script type="module">([\s\S]*?)<\/script>/)[1]
  .replace(/^import .*;\n/gm, "");
async function page({ checksum = hash, supported = true, resume = false, stopped = false } = {}) {
  const elements = new Map();
  function element(id) {
    if (!elements.has(id)) elements.set(id, { disabled: false, checked: false, value: "", textContent: "", files: [],
      listeners: {}, addEventListener(type, callback) { this.listeners[type] = callback; } });
    return elements.get(id);
  }
  let midiCalls = 0, installs = 0, resumes = 0, confirms = 0, accept = true, closes = 0;
  const ports = [{ close: async () => { closes++; } }];
  class Updater {
    async find() { return resume ? { link: { close() {} } } : null; }
    async install(image, product, step) {
      assert.equal(product, "FM-1_900"); assert.ok(image.length > 0); installs++;
      step("verify", 74); step("write", 1167); step("reboot"); step("done", product);
    }
    async resume(image, step, product) { resumes++; assert.equal(product, "FM-1_900"); if (!stopped) step("done", product); return !stopped; }
  }
  const context = { document: { getElementById: element }, window: { addEventListener() {} },
    isSecureContext: true, crypto: webcrypto, navigator: supported ? { requestMIDIAccess: async () => {
      midiCalls++; return { inputs: new Map([[0, ports[0]]]), outputs: new Map() };
    } } : {},
    fetch: async () => ({ ok: true, arrayBuffer: async () => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength) }),
    confirm: () => { confirms++; return accept; }, TextDecoder, Uint8Array, Updater, productOf, logicalImage };
  const meta = { version: "0.1.0", candidate: true, product: "FM-1_900", pkg: "../downloads/ambwave-0.1.0.fwsc", sha256: checksum };
  vm.runInNewContext(script.replace("/*META*/", JSON.stringify(meta)) + "\nthis.loaded = () => selected;", context);
  // WebCrypto completion is asynchronous; wait boundedly for visible state.
  for (let i = 0; i < 100 && element("status").textContent === "Checking the release package…"; i++)
    await new Promise((resolve) => setTimeout(resolve, 5));
  return { element, context, counts: () => ({ midiCalls, installs, resumes, confirms, closes }),
    accept: (value) => { accept = value; }, event: async (id, type) => element(id).listeners[type]() };
}
const ready = await page();
assert.ok(ready.context.loaded()); assert.equal(ready.counts().midiCalls, 0); assert.equal(ready.element("go").disabled, true);
await ready.event("go", "click"); assert.equal(ready.counts().midiCalls, 0);
ready.element("saved").checked = true; await ready.event("saved", "change"); assert.equal(ready.element("go").disabled, false);
ready.accept(false); await ready.event("go", "click"); assert.equal(ready.counts().midiCalls, 0);
ready.accept(true); await ready.event("go", "click");
assert.equal(ready.counts().installs, 1); assert.equal(ready.counts().closes, 1);
assert.match(ready.element("status").textContent, /Update complete.*AMBWAVE/); assert.equal(ready.element("progress").value, 100);
ready.element("previous").files = [{ name: "previous.fwsc", arrayBuffer: async () => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength) }];
await ready.event("previous", "change");
assert.equal(ready.context.loaded().previous, true); assert.match(ready.element("package-info").textContent, /previous.fwsc/);
assert.equal(ready.counts().midiCalls, 1); // loading a rollback file never contacts MIDI
ready.element("previous").files = [{ name: "bad.fwsc", arrayBuffer: async () => new Uint8Array(10).buffer }];
await ready.event("previous", "change");
assert.equal(ready.element("go").disabled, true); assert.equal(ready.context.loaded(), null);
const invalid = await page({ checksum: "0".repeat(64) });
assert.equal(invalid.element("go").disabled, true); assert.match(invalid.element("status").textContent, /checksum mismatch/);
assert.equal(invalid.counts().midiCalls, 0);
const unsupported = await page({ supported: false });
assert.equal(unsupported.element("go").disabled, true); assert.match(unsupported.element("status").textContent, /Chrome or Edge/);
for (const stopped of [false, true]) {
  const resumed = await page({ resume: true, stopped }); resumed.element("saved").checked = true;
  await resumed.event("go", "click"); assert.equal(resumed.counts().resumes, 1); assert.equal(resumed.counts().installs, 0);
  assert.match(resumed.element("status").textContent, stopped ? /update stopped/i : /Update complete/);
}
console.log("AmbWave installer: exact checksum, backup/confirmation gates, unsupported browser, full/resumed install and stopped update passed with fake MIDI");
