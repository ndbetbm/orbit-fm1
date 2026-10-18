import fs from 'node:fs';import assert from 'node:assert/strict';
const {instance}=await WebAssembly.instantiate(fs.readFileSync(process.argv[2]),{});const x=instance.exports;x._initialize();x.web_nor_erase();x.web_boot();
function render(ms){for(let i=0;i<Math.ceil(ms*44.1/128);i++){x.web_render(128);for(const p of [x.web_out_l(),x.web_out_r()])for(const v of new Float32Array(x.memory.buffer,p,128))assert(Number.isFinite(v)&&Math.abs(v)<=1);}}
function snapshot(){return Array.from({length:4},(_,t)=>[x.web_orbit_engine(t),x.web_orbit_preset(t),...Array.from({length:8},(_,k)=>x.web_orbit_sound(t,k))]);}
render(800);const seen=new Set();let runs=0;let varied=0;
for(let seed=2;seed<130;seed++){
 x.web_orbit_regenerate(seed);const a=snapshot();for(const row of a)seen.add(row[0]);
 x.web_orbit_regenerate(seed);assert.deepEqual(snapshot(),a,'same seed independent of previous engine');
 if(new Set(a.map(row=>row[0])).size>1)varied++;
 if(!x.web_playing()){x.web_buttons(1<<10);render(60);x.web_buttons(0);render(60);}
 render(2000);assert.equal(x.web_playing(),1);runs++;
}
assert(seen.size>=12);assert(varied>100);console.log(`PASS: ${runs} seeded scenes, deterministic engine/preset/all 8 sound controls; ${seen.size} active engine types covered (${[...seen].sort((a,b)=>a-b)}); finite bounded audio and continuing playback.`);
