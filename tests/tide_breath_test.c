/* SPDX-License-Identifier: GPL-3.0-only */
/* Opt-in P0a invariants. No factory patch, macro or storage changes. */
#define TIDE_BREATH_EXPERIMENT 1
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

static void blocks(unsigned count)
{
    int32_t out[CTL*2];
    while (count--) mix_block(out, CTL);
}

int main(void)
{
    int fail = 0;
    ambient_audio_reset();
    track_t *t = &trk[0];
    ambient_space_turn(t, -100);
    ambient_motion_turn(t, -100);
    ambient_latch_toggle(t, 50, 100);
    blocks(2000);
    fail |= ambient_check("P0a Motion0 exact unity/no index offset", tide_breath[0][0].gain == 32767);
    uint32_t frequency[6];
    for (unsigned k=0;k<6;k++) frequency[k]=fm6_note[0][0].op[k].freq;
    uint8_t patch[FP_SIZE+1]; memcpy(patch,fm6_patch[0],sizeof patch);
    ambient_motion_turn(t,25);
    int16_t params[P_COUNT]; memcpy(params,t->p,sizeof params);
    int varied=0, previous=tide_breath[0][0].gain;
    for (unsigned b=0;b<60u*FS/CTL;b++) {
        blocks(1);
        varied |= tide_breath[0][0].gain != previous;
        previous=tide_breath[0][0].gain;
        if(previous < 32767-6760 || previous>32767)fail=1;
        for(unsigned k=0;k<6;k++) if(frequency[k]!=(uint32_t)fm6_note[0][0].op[k].freq)fail=1;
    }
    fail |= ambient_check("P0a moves with bounded gain and invariant pitch",varied&&!fail);
    fail |= ambient_check("P0a leaves stored patch and macro parameters intact",
        !memcmp(patch,fm6_patch[0],sizeof patch)&&!memcmp(params,t->p,sizeof params));
    ambient_latch_toggle(t,57,100); ambient_latch_toggle(t,65,100); ambient_latch_toggle(t,72,100);
    blocks(100);
    fail |= ambient_check("P0a independent note phases, same four-note latch cap",
        tide_breath[0][0].phase!=tide_breath[0][1].phase &&
        tide_breath[0][1].phase!=tide_breath[0][2].phase &&
        ambient_latch_toggle(t,74,100)==0 && ambient_gated_notes(t)==4);
    ambient_motion_turn(t,-100); blocks(2);
    fail |= ambient_check("P0a live Motion0 settles to unity",tide_breath[0][0].gain==32767);
    ambient_latch_clear(t); blocks(12u*FS/CTL);
    fail |= ambient_check("P0a releases all FM voices",voices_busy()==0);
    for(unsigned tr=0;tr<NTRK;tr++) for(unsigned sound=AS_TIDE;sound<=AS_CLOUD;sound++) {
        ambient_patch_apply(&trk[tr],sound,0);
        blocks(10);
        ambient_mode_set(&trk[tr],AM_HOLD);
        fail |= ambient_check("P0a only grounded Tide HOLD is eligible",
            (ambient_tide_breath_motion(&trk[tr])!=0)==(sound==AS_TIDE));
        if(sound==AS_TIDE) {
            ambient_mode_set(&trk[tr],AM_SCATTER);
            if(ambient_tide_breath_motion(&trk[tr]))fail=1;
            ambient_mode_set(&trk[tr],AM_SEQUENCE);
            if(ambient_tide_breath_motion(&trk[tr]))fail=1;
            ambient_mode_set(&trk[tr],AM_HOLD);
        }
    }
    ambient_patch_apply(t,AS_TIDE,1);
    fail |= ambient_check("P0a Warm excluded",!ambient_tide_breath_motion(t));
    ambient_patch_apply(t,AS_TIDE,2);
    fail |= ambient_check("P0a Silk excluded",!ambient_tide_breath_motion(t));
    ambient_patch_apply(t,AS_TIDE,0);blocks(10);
    ambient_latch_toggle(t,50,100);
    uint32_t phase=tide_breath[0][0].phase;
    blocks(3000);ambient_latch_clear(t);blocks(12u*FS/CTL);
    ambient_latch_toggle(t,50,100);
    fail |= ambient_check("P0a reused note resets its breath deterministically",
        tide_breath[0][0].phase==phase && tide_breath[0][0].gain==32767);
    printf("P0a state: %zu bytes, no pool allocation\n",sizeof tide_breath);
    return fail;
}
