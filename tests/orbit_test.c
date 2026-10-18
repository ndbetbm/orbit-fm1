/* SPDX-License-Identifier: GPL-3.0-only */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include "../firmware/src/orbit_scene.c"
#define PROJ_HOST 1
#include "../firmware/src/project.c"
#include <assert.h>
static void fresh(void){ memset(trk,0,sizeof trk);memset(&song,0,sizeof song);host_tracks_init();for(unsigned i=0;i<4;i++)host_preset(&trk[i],0,i);orbit_scene(); }
int main(int argc,char **argv){
 fresh();track_t *t=&trk[0];
 for(int g=1;g<=64;g++)for(int h=0;h<=g;h++)for(int r=0;r<g;r++){
  t->p[P_SLEN]=g;t->p[P_OR_DENS]=h;t->p[P_ED_FX]=r;int count=0,first=-1,last=-1,min=100,max=0;
  for(int k=0;k<g;k++)if(orbit_hit(t,k)){count++;if(first<0)first=k;else{int gap=k-last;if(gap<min)min=gap;if(gap>max)max=gap;}last=k;}
  assert(count==h);if(h){int gap=first+g-last;if(gap<min)min=gap;if(gap>max)max=gap;assert(max-min<=1);}
 }
 fresh();t=&trk[0];t->p[P_SLEN]=7;t->p[P_OR_DENS]=64;t->p[P_ED_FX]=38;{int32_t o[2*CTL];mix_block(o,CTL);}assert(t->p[P_OR_DENS]==7&&t->p[P_ED_FX]==3);
 fresh();for(unsigned i=0;i<4;i++){trk[i].p[P_OR_EVOL]=30*(i+1);trk[i].p[P_SLEN]=64;trk[i].p[P_OR_DENS]=1;trk[i].p[P_OR_MODE]=2;assert(orbit_period(&trk[i])==FS*60/(30*(i+1)));}
 song.g[G_BPM]=240;assert(orbit_period(&trk[0])==FS*2);
 transport_req=1;for(unsigned f=0;f<FS*8;f+=CTL){int32_t o[2*CTL];mix_block(o,CTL);}
 for(unsigned i=0;i<4;i++){assert(trk[i].seq_idx>=4*(i+1)-1 && trk[i].seq_idx<=4*(i+1));}
 seq_stop();fresh();
 for(unsigned seed=2;seed<102;seed++){orbit_scene_seed(seed);for(unsigned i=0;i<4;i++)for(unsigned j=0;j<P_COUNT;j++){const param_desc_t *d=param_desc_of(trk[i].eng_req,j);assert(trk[i].p[j]>=d->min&&trk[i].p[j]<=d->max);}}
 orbit_scene_seed(42);int16_t params[P_COUNT];memcpy(params,trk[0].p,sizeof params);orbit_scene_seed(42);assert(!memcmp(params,trk[0].p,sizeof params));orbit_scene_seed(43);assert(memcmp(params,trk[0].p,sizeof params));
 project_t a={0},b;project_store_t st;a.magic=PROJ_MAGIC;a.size=sizeof a;a.parts=NPART;a.phys=PROJ_PHYS;chain_defaults(&a.chain);memcpy(a.g,song.g,sizeof a.g);
 for(unsigned i=0;i<4;i++){a.t[i].engine=trk[i].eng_req;a.t[i].preset=trk[i].preset;memcpy(a.t[i].p,trk[i].p,sizeof trk[i].p);memcpy(a.t[i].step,trk[i].step,sizeof trk[i].step);}
 a.sum=proj_sum(&a);assert(proj_pack(&st,&a)&&proj_import(&b,&st,sizeof st));for(unsigned i=0;i<4;i++)assert(!memcmp(a.t[i].p,b.t[i].p,sizeof a.t[i].p));
 fresh();FILE *w=argc>1?fopen(argv[1],"wb"):0;unsigned frames=FS*48;if(w)wav_hdr(w,frames);transport_req=1;int peak=0;unsigned long long energy=0;
 for(unsigned f=0;f<frames;f+=CTL){int32_t o[2*CTL];mix_block(o,CTL);for(unsigned k=0;k<CTL&&f+k<frames;k++){for(unsigned c=0;c<2;c++){int v=o[2*k+c],vabs=v<0?-v:v;assert(vabs<32768);if(vabs>peak)peak=vabs;energy+=(int64_t)v*v;}if(w)wav_put(w,o[2*k],o[2*k+1]);}}
 if(w)fclose(w);assert(peak>100&&energy>0);seq_stop();
 puts("PASS: every Euclidean grid/hit/rotation up to 64 (exact count and balanced gaps); four independent clocks; 100 random scenes in range; deterministic regeneration; project roundtrip; 48-second 4-voice ambient render without clipped samples.");printf("Peak=%d\n",peak);return 0;
}
