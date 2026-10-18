/* SPDX-License-Identifier: GPL-3.0-only */
static void orbit_scene_seed(uint32_t seed)
{
    static const uint8_t grid[]={13,17,11,19}, hits[]={4,5,3,7}, bpm[]={43,57,31,67};
    for(uint32_t i=0;i<NPART;i++) {
        track_t *t=&trk[i]; orbit_rng[i]=seed^(i+1u)*2246822519u;
        /* Reset sound state so regeneration never inherits the previous engine's settings. */
        if(seed!=1) for(uint32_t j=0;j<P_COUNT;j++) t->p[j]=TP[j].def;
        t->user=0;

        const preset_t *p=&ANALOG_PRESETS[i];
        t->eng_req=0; t->preset=(uint8_t)i;
        for(uint32_t j=0;j<8;j++) t->p[P_E0+j]=p->e[j];
        t->p[P_ATK]=p->env[0];t->p[P_DEC]=p->env[1];t->p[P_SUS]=p->env[2];t->p[P_REL]=p->env[3];
        t->p[P_VOICE]=V_POLY;t->p[P_AMODE]=0;t->p[P_CHRD]=0;
        t->p[P_MUTE]=0; t->p[P_OR_MODE]=1;
        t->p[P_SLEN]=grid[i];t->p[P_OR_DENS]=hits[i];t->p[P_OR_EVOL]=bpm[i];t->p[P_ED_FX]=i;
        t->p[P_SDIV]=0;t->p[P_SGATE]=110;t->p[P_SQNT]=1;t->p[P_SSWING]=0;
        t->p[P_ROOT]=0;t->p[P_SCALE]=5;t->p[P_QUANT]=4;t->p[P_TRANS]=0;
        t->p[P_LEVEL]=76;t->p[P_PAN]=(int16_t)(-36+(int)i*24);
        t->p[P_REV]=120;t->p[P_DLY]=92;t->p[P_CHOR]=104;t->p[P_DIST]=0;
        t->p[P_LRATE]=4+i*3;t->p[P_LTRIG]=1;t->p[P_LSYNC]=0;t->p[P_ESYNC]=0;
        t->p[P_LD_PIT]=0;t->p[P_LD_FLT]=0;t->p[P_LD_SHP]=0;t->p[P_LD_AMP]=0;
        t->p[P_ED_FLT]=0;t->p[P_ED_PIT]=0;t->p[P_ED_SHP]=0;
        t->p[P_M1SRC]=MS_SLEW;t->p[P_M1DST]=MD_PAN;t->p[P_M1AMT]=26;
        t->p[P_M2SRC]=MS_LFO;t->p[P_M2DST]=MD_CUT;t->p[P_M2AMT]=10;
        t->p[P_M3SRC]=MS_SLEW;t->p[P_M3DST]=MD_REV;t->p[P_M3AMT]=6;
        t->p[P_M4SRC]=0;t->p[P_M4DST]=0;t->p[P_M4AMT]=0;t->p[P_ITYPE]=0;
        if(seed!=1) {
            #define OR_R(a,b) ((int32_t)(a)+(int32_t)(orbit_random(t)%((b)-(a)+1)))
            t->p[P_SLEN]=OR_R(5,32);t->p[P_OR_DENS]=OR_R(1,t->p[P_SLEN]/2+1);
            t->p[P_OR_EVOL]=OR_R(18,95);t->p[P_ED_FX]=OR_R(0,t->p[P_SLEN]-1);t->p[P_SDIV]=OR_R(0,2);
            t->p[P_SGATE]=OR_R(65,127);t->p[P_LEVEL]=OR_R(62,84);t->p[P_PAN]=OR_R(-50,50);
            uint32_t engine=eng_vis(orbit_random(t)%NENG_SHOWN);
            const engine_t *e=ENGINES[engine];
            t->eng_req=(uint8_t)engine;
            t->preset=e->npresets?(uint8_t)(orbit_random(t)%e->npresets):0;
            for(uint32_t j=0;j<8;j++) t->p[P_E0+j]=e->npresets?e->presets[t->preset].e[j]:e->edit[j].def;
            for(uint32_t j=0;j<8;j++) {
                const param_desc_t *d=e->desc?e->desc(t,j):0;
                if(!d)d=&e->edit[j];
                t->p[P_E0+j]=OR_R(d->min,d->max);
            }
            /* Only populated FM6 factory slots: empty user-bank slots are not sounds. */
            if(engine==ENGI_FM6)t->p[P_E7]=OR_R(0,FM6_NFACTORY-1);
            fm6_track_loaded(t);
            t->p[P_ATK]=OR_R(0,127);t->p[P_DEC]=OR_R(0,127);t->p[P_SUS]=OR_R(0,127);t->p[P_REL]=OR_R(0,127);
            t->p[P_ED_FLT]=OR_R(-64,63);t->p[P_ED_PIT]=OR_R(-64,63);t->p[P_ED_SHP]=OR_R(-64,63);
            t->p[P_FTYPE]=OR_R(TP[P_FTYPE].min,TP[P_FTYPE].max);
            t->p[P_REV]=OR_R(0,127);t->p[P_DLY]=OR_R(0,127);t->p[P_CHOR]=OR_R(0,127);t->p[P_DIST]=OR_R(0,127);
            t->p[P_LRATE]=OR_R(0,127);t->p[P_LWAVE]=OR_R(TP[P_LWAVE].min,TP[P_LWAVE].max);t->p[P_M1AMT]=OR_R(8,40);t->p[P_M2AMT]=OR_R(3,20);
            t->p[P_TRANS]=OR_R(-12,12);
            #undef OR_R
        }
        for(uint32_t j=0;j<NSTEP;j++) {orbit_generate(t,j);if(!orbit_hit(t,j)){t->step[j].time=ST_REST;t->step[j].n=0;}}
    }
    song.g[G_BPM]=54;song.g[G_CLOCK]=0;song.g[G_SWING]=0;
    song.g[G_RTYPE]=2;song.g[G_RSIZE]=123;song.g[G_RDAMP]=88;
    song.g[G_DTIME]=0;song.g[G_DFDBK]=103;song.g[G_DCOLOR]=40;song.g[G_DMIX]=110;
    song.g[G_CRATE]=18;song.g[G_CDEPTH]=110;
    if(seed!=1){
        song.g[G_RSIZE]=105+orbit_random(&trk[0])%23;song.g[G_RDAMP]=60+orbit_random(&trk[0])%55;
        song.g[G_DFDBK]=80+orbit_random(&trk[0])%39;song.g[G_DTIME]=orbit_random(&trk[0])%3;
        song.g[G_DCOLOR]=20+orbit_random(&trk[0])%55;song.g[G_DMIX]=80+orbit_random(&trk[0])%48;
        song.g[G_CRATE]=5+orbit_random(&trk[0])%35;song.g[G_CDEPTH]=75+orbit_random(&trk[0])%53;
        uint32_t root=orbit_random(&trk[0])%12,scale=orbit_random(&trk[0])%2?5:6;
        for(uint32_t i=0;i<NPART;i++){trk[i].p[P_ROOT]=root;trk[i].p[P_SCALE]=scale;}
    }
}
static void orbit_scene(void) { orbit_scene_seed(1); }

