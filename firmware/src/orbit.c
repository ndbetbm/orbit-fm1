/* SPDX-License-Identifier: GPL-3.0-only
 * ORBIT 0.2: independently clocked Euclidean pulse generators.
 * Reuse the three ORBIT slots: MODE / HITS / BPM. P_ED_FX (unused upstream) is rotation.
 * The step array is a runtime note cache, not the rhythm source. */
static uint32_t orbit_rng[NPART];
static uint32_t orbit_random(track_t *t) {
    uint32_t i=(uint32_t)(t-trk), x=orbit_rng[i];
    if(!x) x=0x6d2b79f5u ^ (i+1u)*2246822519u;
    x^=x<<13; x^=x>>17; x^=x<<5; return orbit_rng[i]=x;
}
static uint32_t orbit_grid(const track_t *t) { return (uint32_t)clamp(t->p[P_SLEN],1,64); }
static int orbit_hit(const track_t *t,uint32_t index) {
    uint32_t grid=orbit_grid(t), hits=(uint32_t)clamp(t->p[P_OR_DENS],0,(int32_t)grid);
    uint32_t rotation=(uint32_t)clamp(t->p[P_ED_FX],0,63)%grid;
    uint32_t i=(index+grid-rotation)%grid;
    return hits && (i*hits)%grid < hits;
}
static uint32_t orbit_period(const track_t *t) {
    static const uint8_t den[]={1,2,4,8,3,6};
    uint32_t q=FS*60u/(uint32_t)clamp(t->p[P_OR_EVOL],10,120);
    uint32_t div=(uint32_t)clamp(t->p[P_SDIV],0,9);
    return div<6?q/den[div]:q<<(div-5);
}
static void orbit_generate(track_t *t,uint32_t i) {
    step_t *s=&t->step[i];
    memset(s,0,sizeof *s); s->time=ST_NOTE; s->n=1;
    s->note[0]=(uint8_t)clamp(scale_snap(t,36+(int32_t)(t-trk)*7+(int32_t)(orbit_random(t)%20)),0,127);
    s->vel=(uint8_t)(58+orbit_random(t)%25);
}
static void orbit_enter(track_t *t) {
    step_t *s=&t->step[t->seq_idx];
    if (!t->p[P_OR_MODE] || !orbit_hit(t,t->seq_idx)) {
        s->time=ST_REST; s->n=0; s->hit=0; return;
    }
    if(!s->n || s->time!=ST_NOTE || (t->p[P_OR_MODE]==1 && orbit_random(t)%100<25)) orbit_generate(t,t->seq_idx);
    /* No step conditions, chance or ratchets may punch holes in a Euclidean rhythm. */
    s->flags=0; s->probability=0; s->hit=0; s->acc=0;
}
