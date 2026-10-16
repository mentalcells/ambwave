/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
static void mode_blocks(uint32_t n) { int32_t out[CTL*2]; while(n--)mix_block(out,CTL); }
int main(void)
{
    int bad = 0;
    for(uint32_t k=0;k<NTRK;k++) {
        ambient_audio_reset(); track_t *c=&trk[k];
        ambient_sound_apply(c,AS_CLOUD,0);ambient_mode_set(c,AM_HOLD);mode_blocks(100);
        ambient_latch_toggle(c,60,100);ambient_latch_toggle(c,64,100);ambient_latch_toggle(c,67,100);
        ambient_latch_toggle(c,71,100);
        bad+=ambient_check("CLOUD HOLD accepts three pitches and bounds a fourth",ambient_layer(c)->count==3&&ambient_gated_notes(c)==3);
        ambient_latch_toggle(c,64,100);ambient_latch_toggle(c,71,100);
        bad+=ambient_check("CLOUD HOLD toggles individual pitches and reuses the freed place",ambient_layer(c)->count==3&&ambient_gated_notes(c)==3);
        ambient_sound_apply(c,AS_TEXTURE,0);mode_blocks(100);ambient_latch_toggle(c,60,100);ambient_latch_toggle(c,64,100);
        bad+=ambient_check("TEXTURE retains its single held register",ambient_layer(c)->count==1&&ambient_gated_notes(c)==1);
    }
    ambient_audio_reset();song.sel=3;track_t *c=&trk[3];
    ambient_mode_set(c,AM_SCATTER);
    bad+=ambient_check("CLOUD FLOW starts with a slow clock and longer envelope",c->p[P_ARATE]==pulse_rates[0]&&c->p[P_ATK]==72&&c->p[P_REL]==80);
    ambient_pulse_speed_turn(2);int rate=c->p[P_ARATE];
    btn_down(B_HOME);frame();press(B_ENV);btn_up(B_HOME);frame();
    bad+=ambient_check("HOME ENV selects LONG without changing explicit speed or leaving HOME",ui.home&&ambient_layer(c)->cloud_length==2&&c->p[P_ARATE]==rate&&c->p[P_AGATE]==127);
    ambient_mode_set(c,AM_SEQUENCE);c->step[0].n=1;c->step[0].note[0]=60;c->step[0].time=ST_NOTE;
    step_t saved[NSTEP];memcpy(saved,c->step,sizeof saved);
    ambient_cloud_length_turn(c);
    bad+=ambient_check("SHORT restores original clocked articulation and preserves the pattern",c->p[P_ATK]==24&&c->p[P_REL]==48&&c->p[P_SGATE]==64&&!memcmp(saved,c->step,sizeof saved));
    ambient_sound_apply(c,AS_TIDE,0);ambient_sound_apply(c,AS_CLOUD,0);ambient_mode_set(c,AM_SCATTER);
    bad+=ambient_check("CLOUD length and explicit scatter speed survive sound changes",ambient_layer(c)->cloud_length==0&&c->p[P_ARATE]==rate);
    ambient_mode_set(c,AM_HOLD);ambient_cloud_length_turn(c);
    bad+=ambient_check("CLOUD HOLD remains sustained when length is pressed",ambient_layer(c)->cloud_length==0&&c->p[P_ATK]==88&&c->p[P_REL]==96);
    for(uint32_t length=0;length<3;length++)for(uint32_t wet=0;wet<2;wet++) {
        ambient_audio_reset();c=&trk[3];ambient_layer(c)->cloud_length=length;
        ambient_layer(c)->space=wet?50:0;ambient_space_apply(c);ambient_mode_set(c,AM_SCATTER);
        ambient_latch_toggle(c,60,100);ambient_latch_toggle(c,64,100);ambient_latch_toggle(c,67,100);seq_start();
        double energy=0;int32_t peak=0;int quiet=1;
        for(uint32_t f=0;f<18u*FS;f+=CTL) {
            if(f/CTL==6u*FS/CTL)seq_stop();
            int32_t out[CTL*2];mix_block(out,CTL);
            for(uint32_t i=0;i<CTL*2;i++) {
                int32_t a=out[i]<0?-out[i]:out[i];if(a>peak)peak=a;
                if(f<6u*FS)energy+=(double)out[i]*out[i];
                if(f>17u*FS&&a>8)quiet=0;
            }
        }
        uint32_t active=0;for(uint32_t v=0;v<NVOICE;v++)active+=c->v[v].active;
        bad+=ambient_check("each CLOUD length is audible, bounded and releases after STOP, dry and wet",energy>0&&peak<32767&&quiet&&!active);
    }
    ambient_audio_reset(); song.sel=3; track_t *t=&trk[3];
    ambient_tap_key(7);
    btn_down(B_HOME); frame(); press(B_ARP); btn_up(B_HOME); frame(); mode_blocks(100);
    bad+=ambient_check("HOME ARP changes CLOUD to SCATTER without leaving HOME or retaining old hold",
        ui.home&&ambient_layer(t)->mode==AM_SCATTER&&!ambient_layer(t)->count&&t->p[P_AMODE]==4);
    ambient_tap_key(7);ambient_tap_key(11);ambient_tap_key(14);
    bad+=ambient_check("CLOUD scatter accepts a pitch set, not a single held register",t->nheld==3&&ambient_layer(t)->count==3);
    seq_start();int allowed=1,heard=0;
    for(uint32_t i=0;i<8u*FS/CTL;i++) {
        mode_blocks(1);
        if(ambient_layer(t)->arp_gate) { heard=1;allowed &= t->arp_note==60||t->arp_note==64||t->arp_note==67; }
    }
    seq_stop();
    bad+=ambient_check("CLOUD scatter plays held pitches and STOP releases its generated gate",heard&&allowed&&!ambient_layer(t)->arp_gate);
    go_home();press(B_SEQ);mode_blocks(100);
    bad+=ambient_check("SEQ from CLOUD HOME enters sequence mode and the existing STEP editor",
        !ui.home&&cur_page()->graph==GR_ROLL&&ambient_layer(t)->mode==AM_SEQUENCE&&t->p[P_AMODE]==0&&t->p[P_SLEN]==8);
    const uint8_t keys[]={7,11,14,9,7,11,14,9},notes[]={60,64,67,62,60,64,67,62};
    int entered=1;
    for(uint32_t i=0;i<8;i++) { key_down(keys[i]);frame();key_up(keys[i]);frame(); entered &= t->step[i].n==1&&t->step[i].note[0]==notes[i]; }
    bad+=ambient_check("physical step entry writes a CLOUD phrase without latching preview notes",
        entered&&!ambient_layer(t)->count&&!ambient_gated_notes(t)&&ui.cursor==0);
    step_t pattern[NSTEP];memcpy(pattern,t->step,sizeof pattern);
    go_home();turn(EN_PRESET,1);mode_blocks(100);
    bad+=ambient_check("sound selection retains SEQUENCE mode and the exact phrase",
        ui.home&&pulse_active(t)&&ambient_layer(t)->mode==AM_SEQUENCE&&t->p[P_AMODE]==0&&!memcmp(pattern,t->step,sizeof pattern));
    turn(EN_PRESET,-1);mode_blocks(100);seq_start();int plays=1;
    uint32_t period=div_samples(t->p[P_SDIV]);
    for(uint32_t i=0;i<8;i++) {
        seq_tick(t,i?period:0);
        plays &= t->seq_idx==i&&t->seq_n==1&&t->seq_notes[0]==notes[i];
    }
    seq_stop();
    bad+=ambient_check("CLOUD sequence follows the written pitch order and STOP releases it",plays&&!t->seq_n&&!ambient_gated_notes(t));
    ambient_mode_set(t,AM_HOLD);ambient_tap_key(7);seq_start();seq_tick(t,period*8);
    bad+=ambient_check("HOLD suppresses saved sequence playback while retaining the pattern",
        !t->seq_n&&ambient_layer(t)->count==1&&!memcmp(pattern,t->step,sizeof pattern));seq_stop();
    ambient_mode_set(t,AM_SEQUENCE);btn_down(B_HOME);frame();turn(EN_SELECT,1);btn_up(B_HOME);frame();
    bad+=ambient_check("HOME SELECT advances sequence speed from the actual division",t->p[P_SDIV]==0);
    /* All sound/mode combinations are reachable without replacing a pattern. */
    for(uint32_t k=0;k<NTRK;k++)for(uint32_t sound=AS_TIDE;sound<=AS_CLOUD;sound++)for(uint32_t mode=0;mode<3;mode++) {
        ambient_mode_set(&trk[k],mode);ambient_sound_apply(&trk[k],sound,0);mode_blocks(100);
        bad+=ambient_check("every track retains its chosen mode across every sound",
            ambient_home_active(&trk[k])&&ambient_layers[k].mode==mode&&trk[k].p[P_AMODE]==(mode==AM_SCATTER?4:0));
    }
    /* Four tracks together: sustained harmony/noise, clocked plucks, written grains. */
    ambient_audio_reset();t=&trk[3];ambient_mode_set(t,AM_SEQUENCE);
    for(uint32_t i=0;i<8;i++){t->step[i].n=1;t->step[i].time=ST_NOTE;t->step[i].note[0]=notes[i];}
    ambient_latch_toggle(&trk[0],48,100);ambient_latch_toggle(&trk[0],55,100);
    ambient_latch_toggle(&trk[0],60,100);ambient_latch_toggle(&trk[0],64,100);
    ambient_latch_toggle(&trk[1],60,100);ambient_latch_toggle(&trk[2],72,100);seq_start();
    int pressure=1;double energy=0;int32_t peak=0;
    for(uint32_t f=0;f<16u*FS;f+=CTL){
        int32_t out[CTL*2];mix_block(out,CTL);
        if(f>FS){
            uint32_t voices=0;for(uint32_t k=0;k<NTRK;k++)for(uint32_t v=0;v<NVOICE;v++)voices+=trk[k].v[v].active;
            pressure &= voices<=NVOICE&&ambient_gated_notes(&trk[0])==4&&ambient_gated_notes(&trk[1])==1;
            for(uint32_t i=0;i<CTL*2;i++){int32_t a=out[i]<0?-out[i]:out[i];if(a>peak)peak=a;energy+=(double)out[i]*out[i];}
        }
    }
    seq_stop();bad+=ambient_check("CLOUD phrase with TIDE/TEXTURE/PULSE stays audible, bounded and within shared voices",pressure&&energy>0&&peak<32767);
    return bad?1:0;
}
