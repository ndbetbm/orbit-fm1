/* SPDX-License-Identifier: GPL-3.0-only */
#include "felucca_web.c"
#include <assert.h>
#include <math.h>
static double tone(int depth,int res,int hz){
 orbit_filter_depth=orbit_filter_res=0;orbit_filter_tick=0;memset(orbit_filter_state,0,sizeof orbit_filter_state);
 song.g[G_DRLVL]=depth;song.g[G_DRREV]=res;double energy=0;
 for(int i=0;i<44100;i++){int32_t in=(int32_t)(12000*sin(6.283185307179586*hz*i/44100)),l=in,r=-in;orbit_filter(&l,&r);assert(abs(l)<1000000&&abs(r)<1000000);if(!depth)assert(l==in&&r==-in);if(i>22050)energy+=(double)l*l;}
 return energy;
}
int main(void){web_nor_erase();web_boot();double dry=tone(0,0,8000),hi=tone(70,0,8000),lo=tone(70,0,200);assert(hi<dry*.1);assert(lo>hi*5);
 for(int d=0;d<=127;d++)for(int r=0;r<=127;r+=127){song.g[G_DRLVL]=d;song.g[G_DRREV]=r;for(int i=0;i<1024;i++){int32_t l=(i&1)?100000:-100000,r=-l;orbit_filter(&l,&r);assert(abs(l)<1000000&&abs(r)<1000000);}}
 puts("PASS: stereo bypass bit-exact; high-frequency attenuation; low-frequency retention; all depth values and maximum resonance stay bounded (ASan).");return 0;}
