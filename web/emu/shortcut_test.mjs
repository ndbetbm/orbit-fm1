import fs from 'node:fs';import assert from 'node:assert/strict';
const {instance}=await WebAssembly.instantiate(fs.readFileSync(process.argv[2]),{});const x=instance.exports;x._initialize();x.web_nor_erase();x.web_boot();
function render(ms){for(let i=0;i<Math.ceil(ms*44.1/128);i++)x.web_render(128);}
function press(mask,ms){x.web_buttons(mask);render(ms);}
function sound(){return Array.from({length:4},(_,t)=>[x.web_orbit_engine(t),...Array.from({length:8},(_,k)=>x.web_orbit_sound(t,k))]);}
render(800);const both=(1<<12)|(1<<13);let before=sound();press(both,1200);press(0,80);assert.deepEqual(sound(),before);
x.web_orbit_scope();press(both,500);press(0,80);assert.deepEqual(sound(),before);
press(both,1200);assert.notDeepEqual(sound(),before);before=sound();press(both,1500);assert.deepEqual(sound(),before);assert.equal(x.web_playing(),0);
press(0,80);press(1<<10,80);press(0,200);assert.equal(x.web_playing(),1);press(both,1200);press(0,80);assert.notDeepEqual(sound(),before);assert.equal(x.web_playing(),1);
console.log('PASS: compiled WASM physical OCT chord regenerates all-track scene only on OSC x4, ignores short holds, fires once per hold and preserves transport.');
