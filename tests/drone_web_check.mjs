// SPDX-License-Identifier: GPL-3.0-only
// Exact native/WebAssembly audio comparison using public panel gestures.
import fs from 'node:fs';
const bytes=fs.readFileSync(process.argv[2]||'build/emu/felucca.wasm');
const native=fs.readFileSync(process.argv[3]||'build/drone/native.f32');
const {instance}=await WebAssembly.instantiate(bytes,{}), e=instance.exports;
e._initialize();e.web_nor_erase();e.web_boot();
let offset=0,record=false,peak=0;
function render(ms) {
  const n=Math.round(ms*44.1);
  for(let k=0;k<n;k+=128) {
    const m=Math.min(128,n-k);e.web_render(m);
    if(!record)continue;
    const l=new Float32Array(e.memory.buffer,e.web_out_l(),m),r=new Float32Array(e.memory.buffer,e.web_out_r(),m);
    for(let i=0;i<m;i++) {
      if(native.readFloatLE(offset)!==l[i]||native.readFloatLE(offset+4)!==r[i])throw Error(`native/Wasm mismatch at frame ${offset/8}`);
      offset+=8;peak=Math.max(peak,Math.abs(l[i]),Math.abs(r[i]));
    }
  }
}
render(1000);record=true;
e.web_buttons(1<<6);render(40);e.web_enc(2,-1);render(40);e.web_buttons(0);render(40);
if(((e.web_mix_state(0)>>>16)&255)!==5)throw Error('Drone role not selected');
e.web_keys((1<<7)|(1<<11));render(60);e.web_keys(0);render(60000);
if(e.web_ambient_latch_count()!==2)throw Error('Drone chord not latched');
e.web_enc(1,1);render(40);e.web_keys(1<<14);render(60);e.web_keys(0);render(1000);
e.web_enc(1,-1);render(40);e.web_keys((1<<9)|(1<<13));render(60);e.web_keys(0);render(1000);
for(let knob=3;knob<=6;knob++){e.web_enc(knob,20);render(40);}
render(3000);
if(offset!==native.length||!peak||peak>=1)throw Error(`audio bounds/length: ${peak} ${offset}/${native.length}`);
console.log(`Drone native/Wasm: ${offset/8} identical stereo frames; peak ${peak}; latch ${e.web_ambient_latch_count()}`);
