import fs from 'node:fs';
import assert from 'node:assert/strict';
const {instance}=await WebAssembly.instantiate(fs.readFileSync(process.argv[2]),{});
const x=instance.exports;x._initialize();x.web_nor_erase();x.web_boot();
function render(ms){for(let i=0;i<Math.ceil(ms*44.1/128);i++)x.web_render(128);}
function press(mask){x.web_buttons(mask);render(80);x.web_buttons(0);render(80);}
const buf=t=>Array.from(new Int16Array(x.memory.buffer,x.web_orbit_scope_data(t),256));
render(800);
for(let t=0;t<4;t++){x.web_orbit_set(t,0,120);x.web_orbit_set(t,1,8);x.web_orbit_set(t,2,8);}
press(1<<10);render(4000);
for(let t=0;t<4;t++)assert(buf(t).some(v=>v!==0),'live track '+t);
assert.notDeepEqual(buf(0),buf(1));
x.web_orbit_scope();render(300);const draws=x.web_screen_draws();render(300);assert(x.web_screen_draws()>draws,'scope refreshes');
for(let t=0;t<4;t++)x.web_orbit_set(t,4,0);
render(60000);
for(let t=0;t<4;t++)assert(buf(t).every(v=>v===0),'silent track clears '+t);
// HOME goes OSC x4 -> CLOCK -> HOME -> MIXER -> OSC x4 without affecting transport.
for(let i=0;i<4;i++)press(1<<6);
assert.equal(x.web_playing(),1);
console.log('PASS: four live independent waveform buffers; silent buffers clear; overlay refreshes; hardware HOME navigation preserves transport.');
