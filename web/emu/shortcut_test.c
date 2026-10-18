#include "felucca_web.c"
#include <assert.h>
static void advance(unsigned ms){for(unsigned i=0;i<(ms*FS/1000+127)/128;i++)web_render(128);}
static void buttons(uint32_t mask,unsigned ms){web_buttons(mask);advance(ms);}
int main(void){
 web_nor_erase();web_boot();advance(800);
 uint32_t both=(1u<<B_OCTDN)|(1u<<B_OCTUP);
 buttons(both,1200);buttons(0,80);assert(orbit_regen_count==0);
 web_orbit_scope();song.octave=2;song.g[G_DRLVL]=30;song.g[G_DRREV]=20;
 buttons(both,500);buttons(0,80);assert(orbit_regen_count==0);
 buttons(1u<<B_OCTDN,100);buttons(both,1200);
 assert(orbit_regen_count==1);assert(!song.playing);assert(song.octave==2);
 assert(song.g[G_DRLVL]==30&&song.g[G_DRREV]==20);
 buttons(both,1500);assert(orbit_regen_count==1);
 buttons(1u<<B_OCTUP,80);buttons(both,1200);assert(orbit_regen_count==1);
 buttons(0,80);buttons(1u<<B_PLAY,80);buttons(0,500);assert(song.playing);
 buttons(both,1200);buttons(0,80);assert(orbit_regen_count==2);assert(song.playing);
 assert(cur_page()->graph==GR_ORBIT4);
 ui.menu=1;buttons(both,1200);buttons(0,80);assert(orbit_regen_count==2);
 puts("PASS: OSC x4 chord timing, staggered press, once per full release, no octave change, stopped/playing state, filter preservation, menu/home exclusion.");
}
