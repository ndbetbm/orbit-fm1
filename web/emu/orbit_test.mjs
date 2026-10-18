// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';
import assert from 'node:assert/strict';
import {storeSectors,restoreSectors} from './worklet.js';
const bytes=fs.readFileSync(process.argv[2]||'build/emu/felucca.wasm');
async function device(saved){const {instance}=await WebAssembly.instantiate(bytes,{});const x=instance.exports;x._initialize();x.web_nor_erase();if(saved)restoreSectors(new Uint8Array(x.memory.buffer,x.web_nor(),x.web_nor_size()),saved);x.web_boot();return x;}
let x=await device();let peak=0;
function render(ms,record=false){const samples=[];for(let i=0;i<Math.ceil(ms*44.1/128);i++){x.web_render(128);const l=new Float32Array(x.memory.buffer,x.web_out_l(),128),r=new Float32Array(x.memory.buffer,x.web_out_r(),128);for(let k=0;k<128;k++){assert(Number.isFinite(l[k])&&Number.isFinite(r[k]));peak=Math.max(peak,Math.abs(l[k]),Math.abs(r[k]));if(record)samples.push(l[k],r[k]);}}return samples;}
function press(mask){x.web_buttons(mask);render(60);x.web_buttons(0);render(60);}
render(800);
assert.deepEqual(Array.from({length:4},(_,t)=>x.web_orbit_value(t,0)),[43,57,31,67]);
x.web_orbit_page(0);render(60);x.web_enc(3,1);render(60);assert.equal(x.web_orbit_value(0,0),44);
x.web_enc(4,1);render(60);assert.equal(x.web_orbit_value(0,1),14);
x.web_enc(5,1);render(60);assert.equal(x.web_orbit_value(0,2),5);
x.web_enc(6,1);render(60);assert.equal(x.web_orbit_value(0,3),1);
// Preset browsing through the physical PRESETS encoder must preserve the sequence.
assert.deepEqual(Array.from({length:4},(_,t)=>x.web_orbit_value(t,4)),[1,1,1,1]);
for(let t=1;t<4;t++)x.web_orbit_set(t,4,0); // isolate the track under test
x.web_orbit_set(0,0,117);x.web_orbit_set(0,1,13);x.web_orbit_set(0,2,13);x.web_orbit_set(0,3,4);x.web_orbit_set(0,5,0);
press(1<<10);
for(const mode of [1,2,0]){
 x.web_orbit_set(0,4,mode);
 const before=Array.from({length:8},(_,i)=>x.web_orbit_value(0,i));
 for(let preset=0;preset<3;preset++){
  x.web_enc(2,1);render(100);
  assert.equal(x.web_playing(),1);
  assert.deepEqual(Array.from({length:8},(_,i)=>x.web_orbit_value(0,i)),before,'preset must retain Euclidean controls');
  const audio=render(4000,true);
  if(mode)assert(audio.some(v=>Math.abs(v)>0.0001),'selected track must still sound after preset change');
 }
}
press(1<<10);
console.log('PASS: FLOW defaults; physical PRESETS changes during playback retain all eight sequencer controls in FLOW/HOLD/PAUSE; isolated track audio continues in FLOW/HOLD.');

// Exact Euclidean mask and rotation, including empty/full, without editing notes.
for(let t=0;t<4;t++)for(let grid=1;grid<=64;grid++){x.web_orbit_set(t,1,grid);for(const hits of [0,Math.floor(grid/2),grid]){x.web_orbit_set(t,2,hits);x.web_orbit_set(t,3,grid-1);assert.equal(Array.from({length:grid},(_,i)=>x.web_orbit_hit(t,i)).reduce((a,b)=>a+b,0),hits);}}
for(let t=0;t<4;t++){x.web_orbit_set(t,0,30*(t+1));x.web_orbit_set(t,1,64);x.web_orbit_set(t,2,64);x.web_orbit_set(t,3,0);x.web_orbit_set(t,4,2);x.web_orbit_set(t,5,0);}
press(1<<10);assert.equal(x.web_playing(),1);render(7800);
for(let t=0;t<4;t++)assert(Math.abs(x.web_orbit_step(t)-4*(t+1))<=1);
render(130000); // let the note cache visit a full cycle after changing the topology
const frozen=Array.from({length:4},(_,t)=>x.web_orbit_signature(t));render(5000);assert.deepEqual(Array.from({length:4},(_,t)=>x.web_orbit_signature(t)),frozen);
x.web_orbit_regenerate(42);render(100);assert.equal(x.web_playing(),1);
const settings=Array.from({length:4},(_,t)=>Array.from({length:8},(_,i)=>x.web_orbit_value(t,i)));
x.web_orbit_regenerate(43);render(100);assert.notDeepEqual(Array.from({length:4},(_,t)=>Array.from({length:8},(_,i)=>x.web_orbit_value(t,i))),settings);
press(1<<10);assert.equal(x.web_playing(),0);x.web_orbit_regenerate(42);render(45000);assert.equal(x.web_playing(),0);
assert.deepEqual(Array.from({length:4},(_,t)=>Array.from({length:8},(_,i)=>x.web_orbit_value(t,i))),settings);
const saved=storeSectors(new Uint8Array(x.memory.buffer,x.web_nor(),x.web_nor_size()));assert(Object.keys(saved).length>0);
x=await device(saved);render(800);assert.deepEqual(Array.from({length:4},(_,t)=>Array.from({length:8},(_,i)=>x.web_orbit_value(t,i))),settings);
for(let seed=2;seed<18;seed++){x.web_orbit_regenerate(seed);if(!x.web_playing())press(1<<10);render(6000);}
assert(peak>0.001&&peak<1);
console.log('PASS: actual UI knob mapping; Euclidean endpoints/rotation; independent 30/60/90/120 BPM clocks; HOLD; regenerate playing/stopped and seed reproducibility; save/reload of all clocks; 16 six-second randomized audio scenes. Peak='+peak.toFixed(5));
if(process.argv[3]){
 x=await device();render(800);x.web_orbit_regenerate(42);render(100);press(1<<10);const samples=render(3000,true);
 const native=fs.readFileSync(process.argv[3]);assert.equal(native.length,samples.length*4);for(let i=0;i<samples.length;i++)assert.equal(samples[i],native.readFloatLE(i*4),'audio sample '+i);
 console.log('PASS: native C / WASM sample-for-sample parity ('+samples.length/2+' stereo frames).');
}
