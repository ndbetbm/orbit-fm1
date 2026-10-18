import fs from 'node:fs';import assert from 'node:assert/strict';
import {storeSectors,restoreSectors} from './worklet.js';
const bytes=fs.readFileSync(process.argv[2]);
async function boot(saved){const {instance}=await WebAssembly.instantiate(bytes,{});const x=instance.exports;x._initialize();x.web_nor_erase();if(saved)restoreSectors(new Uint8Array(x.memory.buffer,x.web_nor(),x.web_nor_size()),saved);x.web_boot();return x;}
let x=await boot();function render(ms){for(let i=0;i<Math.ceil(ms*44.1/128);i++){x.web_render(128);for(const ptr of [x.web_out_l(),x.web_out_r()])for(const s of new Float32Array(x.memory.buffer,ptr,128))assert(Number.isFinite(s)&&Math.abs(s)<=1);}}
render(800);x.web_orbit_scope();render(100);assert.equal(x.web_orbit_filter_value(0),0);x.web_enc(3,35);render(100);x.web_enc(4,40);render(100);
const values=[x.web_orbit_filter_value(0),x.web_orbit_filter_value(1)];assert(values[0]>0&&values[1]>0);
x.web_buttons(1<<10);render(80);x.web_buttons(0);render(6000);assert.equal(x.web_playing(),1);
x.web_enc(2,1);render(200);assert.deepEqual([x.web_orbit_filter_value(0),x.web_orbit_filter_value(1)],values);
x.web_buttons(1<<10);render(80);x.web_buttons(0);render(65000);
const saved=storeSectors(new Uint8Array(x.memory.buffer,x.web_nor(),x.web_nor_size()));x=await boot(saved);render(800);assert.deepEqual([x.web_orbit_filter_value(0),x.web_orbit_filter_value(1)],values);
console.log('PASS: OSC x4 hardware knobs adjust global filter; playback and preset changes preserve settings; audio bounded; save/reboot restores both controls. Values='+values);
