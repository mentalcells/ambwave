// SPDX-License-Identifier: GPL-3.0-only
// Native/WASM sample parity, real voice limits, release, nonzero audio and fresh-instance panic.
import fs from 'node:fs';
import assert from 'node:assert/strict';
import {execFileSync} from 'node:child_process';
const module=new WebAssembly.Module(fs.readFileSync('build/sound-bath/integrated.wasm'));
assert.deepEqual(WebAssembly.Module.imports(module),[]);
function device(patch,wet){const ex=new WebAssembly.Instance(module,{}).exports;ex._initialize();assert.equal(ex.sbb_init(patch,wet),1);return ex;}
let checks=0;
for(const patch of [0,3,5])for(const wet of [0]){
 const path=`build/sound-bath/parity-${patch}-${wet}.wav`;
 const r=JSON.parse(execFileSync('build/sound-bath/integrated',[String(patch),String(wet),'pressure',path],{encoding:'utf8'}));
 assert.equal(r.voice_cap,patch<2?6:3);assert.equal(r.max_voices,r.voice_cap);assert.equal(r.end_voices,0);assert(r.peak>100);assert(r.peak<32767);
 const wav=fs.readFileSync(path),frames=(wav.length-44)/4;
 const ex=device(patch,wet),samples=new Float32Array(ex.memory.buffer,ex.sbb_output(),2048);
 assert.equal(ex.sbb_engine_bytes(),r.engine_bytes);
 for(let j=0;j<10;j++)ex.sbb_note(48+j,1,80);
 let off=false,mismatches=0;
 for(let f=0;f<frames;f+=128){
  if(f>=5*44100&&!off){ex.sbb_release();off=true;}
  const n=Math.min(128,frames-f);ex.sbb_render(n);
  for(let i=0;i<2*n;i++)if(Math.trunc(samples[i]*32768)!==wav.readInt16LE(44+f*4+i*2))mismatches++;
 }
 assert.equal(mismatches,0,`native/WASM parity patch${patch} wet${wet}`);assert.equal(ex.sbb_active(),0);
 const fresh=device(patch,wet);fresh.sbb_render(128);assert(new Float32Array(fresh.memory.buffer,fresh.sbb_output(),256).every(v=>v===0),'fresh instance panic silence');
 fs.unlinkSync(path);checks++;
 console.log(`patch ${patch}, wet ${wet}: sample-exact native/WASM; voice cap ${r.voice_cap}; release and panic silence pass`);
}
// Original patch controls and the explicit variant's independent settings.
for(const patch of [3,5]){
 const ex=device(patch,0);assert.equal(ex.sbb_patch_value(6),patch===3?90:0);
 ex.sbb_note(48,1,80);ex.sbb_excitation(80);
 for(let i=0;i<100;i++)ex.sbb_render(128);
 assert.equal(ex.sbb_patch_value(6),80);assert.equal(ex.sbb_active(),1);
 ex.sbb_release();for(let i=0;i<5000;i++)ex.sbb_render(128);assert.equal(ex.sbb_active(),0);
}
assert.equal(device(3,0).sbb_patch_value(6),90,'factory remains untouched');
console.log(`${checks} parity cases + continuous-excitation lifecycle passed`);
