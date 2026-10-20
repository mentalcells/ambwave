/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
static void blocks(unsigned n) { int32_t out[CTL*2]; while(n--)mix_block(out,CTL); }
int main(void)
{
    int bad=0;
    for(unsigned k=0;k<NTRK;k++)for(unsigned patch=8;patch<=9;patch++) {
        ambient_audio_reset();track_t *t=&trk[k];
        ambient_patch_apply(t,AS_TIDE,patch);blocks(100);
        const preset_t *p=&ENGINES[ENGI_PHYS]->presets[3];
        for(unsigned j=0;j<8;j++)bad+=ambient_check("exact source PHYS record or labeled BOWL override",t->p[P_E0+j]==(patch==9&&j==6?0:p->e[j]));
        for(unsigned mode=0;mode<3;mode++) {
            ambient_mode_set(t,mode);
            bad+=ambient_check("mode preserves source attack and release",t->p[P_ATK]==(patch==9?0:p->env[0])&&t->p[P_REL]==(patch==9?100:p->env[3]));
        }
        ambient_mode_set(t,AM_HOLD);
        ambient_latch_toggle(t,48,80);blocks(100);
        bad+=ambient_check("Tide latch holds the resonator",gate_note(t,48)&&ambient_layers[k].count==1);
        ambient_latch_toggle(t,48,80);blocks(10);
        bad+=ambient_check("second strike unlatches the owned note",!gate_note(t,48)&&!ambient_layers[k].count);
        ambient_density_turn(t,-100);blocks(220);
        bad+=ambient_check("excitation slews exactly to zero",!t->p[P_E6]);
        ambient_density_turn(t,100);blocks(220);
        bad+=ambient_check("excitation slews exactly to 100",t->p[P_E6]==100);
        ambient_shape_turn(t,100);ambient_motion_turn(t,100);ambient_space_turn(t,100);blocks(220);
        bad+=ambient_check("four macros remain bounded and effective",t->p[P_E2]==90&&t->p[P_LD_FLT]==8&&t->p[P_REV]==64&&!t->p[P_DLY]&&!t->p[P_CHOR]);
        ambient_patch_apply(t,AS_TIDE,0);blocks(100);
        bad+=ambient_check("patch switch clears latch and restores original Tide",!ambient_layers[k].count&&!ambient_bath_patch(t)&&t->eng_req==ENGI_FM6);
    }
    for(unsigned corner=0;corner<16;corner++) {
        ambient_audio_reset();
        for(unsigned k=0;k<NTRK;k++) {
            track_t *t=&trk[k];ambient_patch_apply(t,k<2?AS_TIDE:AS_PULSE,k<2?8+(k&1):3);
            ambient_shape_turn(t,corner&1?100:-100);ambient_motion_turn(t,corner&2?100:-100);
            ambient_density_turn(t,corner&4?100:-100);ambient_space_turn(t,corner&8?100:-100);
            if(k>=2)ambient_mode_set(t,AM_SCATTER);
            t->p[P_LEVEL]=64;blocks(100);
            ambient_muted = 0;
            for(unsigned j=0;j<4;j++)ambient_latch_toggle(t,48+k*7+j*4,80);
        }
        seq_start();int32_t out[CTL*2],peak=0;double energy=0,dc=0;unsigned voices=0;
        for(unsigned f=0;f<8*FS;f+=CTL) {
            mix_block(out,CTL);unsigned active=0;
            for(unsigned k=0;k<NTRK;k++)for(unsigned j=0;j<NVOICE;j++)active+=trk[k].v[j].active!=0;
            if(active>voices)voices=active;
            for(unsigned j=0;j<CTL*2;j++){int a=abs(out[j]);if(a>peak)peak=a;energy+=(double)out[j]*out[j];dc+=out[j];}
        }
        printf("bath scene corner %u peak %d voices %u DC %.2f\n",corner,peak,voices,dc/(16*FS));
        bad+=ambient_check("combined scene and stealing stay audible within headroom and global budget",peak>100&&peak<32767&&voices<=NVOICE&&energy>1e6&&fabs(dc/(16*FS))<500);
        seq_stop();for(unsigned k=0;k<NTRK;k++)ambient_latch_clear(&trk[k]);blocks(30000);
        unsigned active=0;for(unsigned k=0;k<NTRK;k++)for(unsigned j=0;j<NVOICE;j++)active+=trk[k].v[j].active!=0;
        bad+=ambient_check("release and clearing drain every voice under pressure",!active);
    }
    return bad!=0;
}
