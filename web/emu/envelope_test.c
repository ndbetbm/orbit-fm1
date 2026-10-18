/* SPDX-License-Identifier: GPL-3.0-only */
#include "felucca_web.c"
#include <assert.h>
#include <stdio.h>
static int32_t buf[CTL];
static track_t *setup(int engine, int step) {
 web_nor_erase(); web_boot();
 track_t *t=&trk[0];
 set_engine_of(t,engine);
 for(int i=0;i<256;i++){engine_block(t);track_render(t,buf,CTL);}
 memset(t->v,0,sizeof t->v);
 t->p[P_MUTE]=0;t->p[P_ESYNC]=0;t->p[P_LD_AMP]=0;
 t->p[P_ATK]=127;t->p[P_DEC]=0;t->p[P_SUS]=0;t->p[P_REL]=0;
 if(step){t->p[P_E0]=CHIP_STEP;t->p[P_E3]=0;}
 trk_note_on(t,60,127);
 return t;
}
static double onset(int engine, int step, int attack) {
 track_t *t=setup(engine,step);t->p[P_ATK]=attack;t->p[P_SUS]=127;
 double energy=0;
 for(int b=0;b<32;b++){track_render(t,buf,CTL);for(int i=0;i<CTL;i++)energy+=(double)buf[i]*buf[i];}
 return energy;
}
int main(void){
 for(int e=0;e<NENGINES;e++){
  if(e==ENGI_DIGITAL)continue;
  for(int step=0;step<(e==3?2:1);step++){
   double fast=onset(e,step,0),slow=onset(e,step,127);
   if(!(fast>0 && slow<fast*.1)){fprintf(stderr,"Audio ATK engine %d step %d: %f / %f\n",e,step,slow,fast);return 1;}
   track_t *t=setup(e,step);voice_t *v=0;
   for(int j=0;j<NVOICE;j++)if(t->v[j].active){v=&t->v[j];break;}
   assert(v);
   track_render(t,buf,CTL);
   if(v->env> (1<<20)){fprintf(stderr,"ATK bypass engine %d step %d\n",e,step);return 1;}
   /* Seed a held voice at full level: DEC/SUS must reduce its gain, not reset it. */
   v->env=1<<24;v->stage=2;
   for(int i=0;i<64;i++)track_render(t,buf,CTL);
   assert(v->env < (1<<16));assert(v->env_out < 256);
   /* A short release must finish even for self-enveloped / one-shot engines. */
   v->active=1;v->env=1<<24;v->stage=2;v->gate=1;
   trk_note_off(t,60);assert(v->stage==3);
   for(int i=0;i<128;i++)track_render(t,buf,CTL);
   if(v->active){fprintf(stderr,"REL bypass engine %d step %d\n",e,step);return 1;}
  }
 }
 puts("PASS: all 13 engines plus LOFI STEP honor slow attack, decay to zero sustain and release completion.");
 return 0;
}
