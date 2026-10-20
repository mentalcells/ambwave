// SPDX-License-Identifier: GPL-3.0-only
// Candidate's actual ambient panel gestures, 40 s, compared with native C.
import fs from 'node:fs';
const {instance}=await WebAssembly.instantiate(fs.readFileSync(process.argv[2]),{});
const ex=instance.exports;ex._initialize();
let recording=false,at=0;
const expected=new Float32Array(fs.readFileSync(process.argv[3]).buffer);
function take(ms) {
  const frames=ms*441/10;
  for(let f=0;f<frames;f+=128) {
    const n=Math.min(128,frames-f);ex.web_render(n);
    if(recording) {
      const l=new Float32Array(ex.memory.buffer,ex.web_out_l(),n);
      const r=new Float32Array(ex.memory.buffer,ex.web_out_r(),n);
      for(let i=0;i<n;i++)for(const x of [l[i],r[i]]) {
        if(x!==expected[at])throw Error(`Native/WASM differ at sample ${at}`);
        at++;
      }
    }
  }
}
ex.web_nor_erase();ex.web_boot();take(1000);
ex.web_enc(6,-100);take(40);
ex.web_keys((1<<7)|(1<<11)|(1<<14)|(1<<18));take(60);ex.web_keys(0);take(60);
recording=true;take(20000);
ex.web_enc(4,100);take(10000);
ex.web_enc(4,-100);take(10000);
if(at!==expected.length)throw Error('Native/WASM length mismatch');
console.log(`P0a native/WASM: ${at/2} dry held-chord frames bit-identical; Motion25/100/0 through panel.`);
