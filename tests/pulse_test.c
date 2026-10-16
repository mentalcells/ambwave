/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

static void pulse_blocks(uint32_t blocks)
{
    int32_t out[CTL * 2];
    while (blocks--) { mix_block(out, CTL); }
}
static int pulse_timing(void)
{
    int bad = 0;
    uint32_t d, b, counts[4];
    for (d = 0; d < 4; d++) {
        ambient_audio_reset(); song.sel = 2; song.g[G_BPM] = 120;
        ambient_layers[2].density = 100; ambient_density_apply(&trk[2]);
        ambient_pulse_speed_turn((int32_t)d - ambient_layers[2].speed);
        ambient_latch_toggle(&trk[2], 60, 100); seq_start(); counts[d] = 0;
        uint32_t previous = 0;
        for (b = 0; b < 16u * FS / CTL; b++) {
            int32_t out[CTL*2]; mix_block(out, CTL);
            if (ambient_layers[2].arp_gate && !previous) counts[d]++;
            previous = ambient_layers[2].arp_gate;
        }
        uint32_t expected = 16u << d;
        printf("pulse speed %s: %u events, expected %u\n", pulse_rate_names[d], counts[d], expected);
        bad += ambient_check("speed produces the documented clocked event count", counts[d] == expected);
    }
    for (d = 1; d < 4; d++) bad += ambient_check("speed event counts strictly increase", counts[d] > counts[d-1]);
    uint32_t previous = 0;
    for (d = 0; d <= 100; d += 25) {
        ambient_audio_reset(); ambient_layers[2].density = d; ambient_density_apply(&trk[2]);
        ambient_latch_toggle(&trk[2], 60, 100); seq_start(); rng_state = 0x1234567u;
        uint32_t hits = 0, period = div_samples(trk[2].p[P_ARATE]);
        for (b = 0; b < 1024; b++) { arp_step(&trk[2], period, period); hits += ambient_layers[2].arp_gate; }
        printf("pulse density %u: %u / 1024 ticks\n", d, hits);
        bad += ambient_check("density changes hit probability without changing speed", ambient_layers[2].speed == 2 && trk[2].p[P_ARATE] == 1 && hits > previous);
        if (!d) bad += ambient_check("zero density remains occasional", hits > 0 && hits < 80);
        if (d == 100) bad += ambient_check("full density plays every tick", hits == 1024);
        previous = hits;
    }
    ambient_layers[2].density = 50; ambient_density_apply(&trk[2]); int32_t probability = trk[2].p[P_APROB];
    ambient_density_turn(&trk[2], 1);
    bad += ambient_check("a single density detent changes probability", trk[2].p[P_APROB] > probability);
    return bad;
}

static int pulse_mute(void)
{
    int bad = 0; track_t *p = &trk[2]; uint32_t k;
    ambient_audio_reset(); usb.config = 1; mo_r = mo_w = 0;
    ambient_latch_toggle(&trk[0],48,100); ambient_latch_toggle(&trk[1],60,100);
    ambient_latch_toggle(p,72,100); seq_start(); pulse_blocks(100);
    int16_t levels[3] = {trk[0].p[P_LEVEL],trk[1].p[P_LEVEL],p->p[P_LEVEL]};
    uint32_t midi_before = mo_w;
    /* HOME keys, like MIX knobs, also win over a quick-layer overlay. */
    ui.layer = LAYER_FX;
    /* No UI frame between HOME and key: capture at the audio boundary. */
    btn_down(B_HOME);
    for (k = 0; k < 3; k++) {
        ambient_tap_key(1 + 2*k);
        bad += ambient_check("HOME black key toggles exactly its layer mute", ambient_muted == (1u << (k+1)) - 1);
    }
    ambient_tap_key(0); frame();
    uint32_t br = 0, leds = key_leds(&br);
    bad += ambient_check("HOME mute LEDs identify all three muted layers", (leds & 0x2au) == 0x2au && (br & 0x2au) == 0);
    pulse_blocks(10); btn_up(B_HOME); frame();
    bad += ambient_check("mutes latch after HOME release without new notes or MIDI", ambient_muted == 7 && mo_w == midi_before && ambient_layers[0].count == 1 && ambient_layers[1].count && ambient_layers[2].count == 1);
    bad += ambient_check("muting preserves levels, gates and transport", trk[0].p[P_LEVEL] == levels[0] && trk[1].p[P_LEVEL] == levels[1] && p->p[P_LEVEL] == levels[2] && gate_note(&trk[0],48) && gate_note(&trk[1],60) && song.playing);
    bad += ambient_check("muted audio ramps fully to zero before the buses", !pf.mg[0] && !pf.mg[1] && !pf.mg[2]);
    uint32_t idx = p->arp_idx; pulse_blocks(2000);
    bad += ambient_check("PULSE keeps advancing its clock while muted", p->arp_idx > idx && p->nheld == 1);
    btn_down(B_HOME); frame();
    for (k = 0; k < 3; k++) ambient_tap_key(1 + 2*k);
    leds = key_leds(&br);
    bad += ambient_check("unmuted HOME keys breathe as available toggles", !leds && (br & 0x2au) == 0x2au);
    btn_up(B_HOME); frame(); pulse_blocks(10);
    bad += ambient_check("unmute restores sustained layers without replaying notes", !ambient_muted && pf.mg[0] == 32768 && pf.mg[1] == 32768 && gate_note(&trk[0],48) && gate_note(&trk[1],60));
    /* Prove audible mute/resume separately from wet tails. */
    ambient_audio_reset(); trk[0].p[P_DLY] = trk[0].p[P_REV] = trk[0].p[P_CHOR] = 0;
    ambient_latch_toggle(&trk[0],60,100); pulse_blocks(2000);
    int32_t out[CTL*2]; uint64_t before = 0, silent = 0, after = 0;
    for (k=0;k<20;k++) {mix_block(out,CTL);for(uint32_t j=0;j<CTL*2;j++)before += abs(out[j]);}
    btn_down(B_HOME); ambient_tap_key(1); btn_up(B_HOME); pulse_blocks(FS/CTL);
    for (k=0;k<20;k++) {mix_block(out,CTL);for(uint32_t j=0;j<CTL*2;j++)silent += abs(out[j]);}
    btn_down(B_HOME); ambient_tap_key(1); btn_up(B_HOME); pulse_blocks(FS/CTL);
    for (k=0;k<20;k++) {mix_block(out,CTL);for(uint32_t j=0;j<CTL*2;j++)after += abs(out[j]);}
    printf("mute audio absolute sum: before %llu muted %llu after %llu mask %u gain %d\n", (unsigned long long)before, (unsigned long long)silent, (unsigned long long)after, ambient_muted, pf.mg[0]);
    bad += ambient_check("HOME mute silences an existing drone and unmute resumes it", before > 100000 && silent < 20u*CTL*2u*8u && after > 100000);
    ambient_audio_reset();btn_down(B_HOME);ambient_tap_key(1);btn_up(B_HOME);
    ambient_latch_toggle(&trk[0],60,100);mix_block(out,CTL);
    uint32_t leak=0;for(k=0;k<CTL*2;k++)leak|=out[k]!=0;
    bad += ambient_check("muting an idle layer also silences its first future note", !leak && !pf.mg[0] && gate_note(&trk[0],60));
    ambient_audio_reset(); return bad;
}

static int pulse_variation(void)
{
    int bad = 0; track_t *p = &trk[2]; double variation[2], stereo[2];
    for (uint32_t pass=0;pass<2;pass++) {
        ambient_audio_reset();ambient_layers[2].motion=pass?100:0;ambient_motion_apply(p);
        ambient_layers[2].space=0;ambient_space_apply(p);mod_seed=0x2545F491u;
        double sum=0,sum2=0,lr=0,total=0;
        for(uint32_t hit=0;hit<16;hit++) {
            trk_note_on(p,60,100);double energy=0;
            for(uint32_t f=0;f<FS/2;f+=CTL) {
                if(f<FS/16&&f+CTL>=FS/16)trk_all_off(p);
                int32_t out[CTL*2];mix_block(out,CTL);
                for(uint32_t j=0;j<CTL;j++) {
                    double l=out[2*j],r=out[2*j+1];energy+=l*l+r*r;
                    lr+=fabs(l-r);total+=fabs(l)+fabs(r);
                }
            }
            double rms=sqrt(energy/(FS));sum+=rms;sum2+=rms*rms;
        }
        double mean=sum/16;variation[pass]=sqrt(fmax(0,sum2/16-mean*mean))/mean;
        stereo[pass]=lr/total;
        printf("PULSE MOTION %u: hit RMS variation %.3f stereo difference %.3f\n",pass?100:0,variation[pass],stereo[pass]);
    }
    bad += ambient_check("MOTION makes repeated identical pitches vary audibly in level/tone",variation[0]<0.02&&variation[1]>0.07);
    bad += ambient_check("MOTION adds stereo variation without pitch modulation",stereo[0]<0.001&&stereo[1]>0.08&&!p->p[P_LD_PIT]);
    for(uint32_t timbre=0;timbre<3;timbre++) {
        double late[2];
        for(uint32_t bright=0;bright<2;bright++) {
            ambient_audio_reset();ambient_layers[2].timbre=timbre;ambient_pulse_timbre_apply(p);
            ambient_layers[2].shape=bright?100:0;ambient_shape_turn(p,0);
            ambient_layers[2].motion=0;ambient_motion_apply(p);ambient_layers[2].space=0;ambient_space_apply(p);
            char path[160];snprintf(path,sizeof path,"build/pulse_demo/%s-shape-%u.wav",pulse_timbre_names[timbre],ambient_layers[2].shape);
            FILE *wav=fopen(path,"wb");if(wav)wav_hdr(wav,(FS/CTL)*CTL);
            trk_note_on(p,60,100);double energy=0;uint32_t n=0;
            for(uint32_t f=0;f+CTL<=FS;f+=CTL) {
                if(f<FS/16&&f+CTL>=FS/16)trk_all_off(p);
                int32_t out[CTL*2];mix_block(out,CTL);
                for(uint32_t j=0;j<CTL;j++) {
                    if(wav)wav_put(wav,out[2*j],out[2*j+1]);
                    if(f>=FS/10&&f<FS/4){energy+=(double)out[2*j]*out[2*j]+(double)out[2*j+1]*out[2*j+1];n+=2;}
                }
            }
            if(wav)fclose(wav);late[bright]=sqrt(energy/n);
        }
        printf("PULSE %s SHAPE: damped tail %.1f ringing tail %.1f\n",pulse_timbre_names[timbre],late[0],late[1]);
        bad+=ambient_check("SHAPE clearly changes damped versus ringing envelope",late[1]>late[0]*2&&late[1]>10);
    }
    return bad;
}

static int pulse_random_notes(void)
{
    uint32_t b, seen = 0; int safe = 1;
    ambient_audio_reset();ambient_layers[2].density = 100;ambient_density_apply(&trk[2]);
    ambient_latch_toggle(&trk[2],60,100);ambient_latch_toggle(&trk[2],67,100);
    ambient_latch_toggle(&trk[2],72,100);seq_start();
    for(b=0;b<8u*FS/CTL;b++) {
        int32_t out[CTL*2];mix_block(out,CTL);
        if(ambient_layers[2].arp_gate) {
            uint32_t n=trk[2].arp_note;
            safe &= n==60 || n==67 || n==72;
            seen |= n==60 ? 1u : n==67 ? 2u : n==72 ? 4u : 0u;
        }
    }
    return ambient_check("random PULSE uses only held pitches and reaches all inputs",safe&&seen==7);
}
static int pulse_audio(void)
{
    int bad = 0; uint32_t c, f, i;
    int32_t maxpeak = 0; double maxdc = 0, maxlate = 0;
    for (uint32_t variant = 0; variant < 3; variant++) for (c = 0; c < 19; c++) {
        ambient_audio_reset();
        ambient_layers[2].timbre = variant; ambient_pulse_timbre_apply(&trk[2]);
        if (c < 16) {
            song.sel=2; ambient_pulse_speed_turn(1); /* fastest clock at every macro corner */
            ambient_layers[2].shape = c & 1 ? 100 : 0; ambient_shape_turn(&trk[2], 0);
            ambient_layers[2].motion = c & 2 ? 100 : 0; ambient_motion_apply(&trk[2]);
            ambient_layers[2].density = c & 4 ? 100 : 0; ambient_density_apply(&trk[2]);
            ambient_layers[2].space = c & 8 ? 100 : 0; ambient_space_apply(&trk[2]);
            trk[0].p[P_LEVEL] = trk[1].p[P_LEVEL] = trk[2].p[P_LEVEL] = 127;
        }
        FILE *wav = 0; char path[120];
        if (c >= 16) {
            const char *names[] = {"pulse-default", "three-layers", "pulse-dense"};
            snprintf(path, sizeof path, "build/pulse_demo/%s-%s.wav", pulse_timbre_names[variant], names[c-16]);
            wav = fopen(path, "wb"); if (wav) wav_hdr(wav, (12u*FS/CTL)*CTL);
            if (c == 18) { ambient_layers[2].density = 100; ambient_density_apply(&trk[2]); }
        }
        if (c < 16 || c == 17) {
            ambient_latch_toggle(&trk[0],48,100); ambient_latch_toggle(&trk[0],55,100);
            ambient_latch_toggle(&trk[0],60,100); ambient_latch_toggle(&trk[0],64,100);
            ambient_latch_toggle(&trk[1],60,100);
        }
        ambient_latch_toggle(&trk[2],60,100); ambient_latch_toggle(&trk[2],67,100);
        ambient_latch_toggle(&trk[2],72,100); ambient_latch_toggle(&trk[2],76,100);
        seq_start(); double sum=0, energy=0, late=0; uint32_t count=0, nl=0;
        for (f=0; f+CTL<=12u*FS; f+=CTL) {
            if (f < 6u*FS && f+CTL >= 6u*FS) {
                seq_stop(); ambient_latch_clear(&trk[0]); ambient_latch_clear(&trk[1]); ambient_latch_clear(&trk[2]);
            }
            int32_t out[CTL*2]; mix_block(out,CTL);
            for(i=0;i<CTL*2;i++) {
                int32_t a=out[i]<0?-out[i]:out[i]; if(a>maxpeak)maxpeak=a;
                if(f>=2u*FS && f<6u*FS) {sum+=out[i];energy+=(double)out[i]*out[i];count++;}
                if(f>=11u*FS) {late+=(double)out[i]*out[i];nl++;}
            }
            if (wav) for (i=0;i<CTL;i++) wav_put(wav,out[2*i],out[2*i+1]);
        }
        if (wav) fclose(wav);
        double dc=fabs(sum/count), tail=sqrt(late/nl);
        if(c>=16) {
            printf("pulse demo %s RMS %.1f DC %.1f late %.1f\n",path,sqrt(energy/count),dc,tail);
            bad+=ambient_check("PULSE demo is audible with a decaying tail",sqrt(energy/count)>30&&tail<500);
        }
        if(dc>maxdc)maxdc=dc; if(tail>maxlate)maxlate=tail;
    }
    printf("pulse corners: peak %d DC %.1f late RMS %.1f\n",maxpeak,maxdc,maxlate);
    bad += ambient_check("three-layer macro corners have headroom, bounded DC and tails",maxpeak<32767 && maxdc<250 && maxlate<500);
    return bad;
}
int main(void)
{
    int bad=0; uint32_t i; track_t *p=&trk[2];
    ambient_audio_reset(); song.sel=2;
    bad+=ambient_check("PULSE boots silent with empty input and CLOUD available on track 4",pulse_active(p)&&!ambient_layers[2].count&&!p->nheld&&!p->p[P_MUTE]&&cloud_active(&trk[3]));
    for(i=7;i<11;i++)ambient_tap_key(i);
    pulse_blocks(500);
    bad+=ambient_check("four physical inputs queue silently while stopped",ambient_layers[2].count==4&&p->nheld==4&&!ambient_gated_notes(p));
    bad+=ambient_check("fifth physical input is rejected",ambient_latch_toggle(p,90,100)==0&&ambient_layers[2].count==4);
    ambient_tap_key(7);
    bad+=ambient_check("same physical pitch toggles off",ambient_layers[2].count==3&&p->nheld==3);
    ambient_latch_clear(p); ambient_latch_toggle(p,60,100);
    ambient_latch_toggle(&trk[0],48,100); ambient_latch_toggle(&trk[1],60,100);
    ambient_layers[2].density=100;ambient_density_apply(p);
    seq_start(); pulse_blocks(1);
    bad+=ambient_check("PLAY generates one pluck without preset steps",p->arp_note==60&&ambient_gated_notes(p)==1&&!p->seq_n);
    seq_stop(); pulse_blocks(2);
    bad+=ambient_check("STOP retains inputs and both sustained holds",ambient_layers[2].count==1&&p->nheld==1&&!ambient_gated_notes(p)&&gate_note(&trk[0],48)&&gate_note(&trk[1],60));
    seq_start(); pulse_blocks(1);
    bad+=ambient_check("restart immediately retriggers retained input",p->arp_note==60&&ambient_gated_notes(p)==1);
    ambient_latch_clear(p);
    bad+=ambient_check("PULSE clear releases generated voice and keeps other layers",!ambient_layers[2].count&&!p->nheld&&!p->arp_note&&!ambient_gated_notes(p)&&gate_note(&trk[0],48));
    seq_stop(); ambient_latch_toggle(p,60,100);
    midi_note_event(2,60,100); midi_note_event(2,60,0);
    bad+=ambient_check("MIDI release preserves physical input owner",ambient_layers[2].count==1&&p->nheld==1);
    midi_note_event(2,60,100); ambient_latch_toggle(p,60,100);
    bad+=ambient_check("physical toggle preserves incoming MIDI input",!ambient_layers[2].count&&p->nheld==1);
    midi_note_event(2,60,0);
    bad+=ambient_check("last MIDI owner removes input",!p->nheld);
    int16_t global[G_COUNT], other[2][P_COUNT]; memcpy(global,song.g,sizeof global);
    memcpy(other[0],trk[0].p,sizeof other[0]);memcpy(other[1],trk[1].p,sizeof other[1]);
    ambient_space_turn(p,100); ambient_motion_turn(p,100); ambient_density_turn(p,100); ambient_shape_turn(p,100);
    bad+=ambient_check("PULSE macros preserve global FX and other sound parameters",!memcmp(global,song.g,sizeof global)&&!memcmp(other[0],trk[0].p,sizeof other[0])&&!memcmp(other[1],trk[1].p,sizeof other[1])&&ambient_layers[0].density==50&&ambient_layers[1].density==25);
    ambient_latch_toggle(p,60,100); seq_start(); pulse_blocks(1); panic_req|=4; pulse_blocks(1);
    bad+=ambient_check("panic clears input ledger and generated gates",!ambient_layers[2].count&&!p->nheld&&!p->arp_note&&!ambient_gated_notes(p));
    ambient_latch_toggle(p,60,100); set_engine_of(p,0); events_block(CTL);
    bad+=ambient_check("sound load leaves curated mode with no held input",!pulse_active(p)&&!ambient_layers[2].count&&!p->nheld);
    /* Repeated dense arp + chord/register changes must keep five sustained gates. */
    ambient_audio_reset(); ambient_layers[2].density=100;ambient_density_apply(p);song.sel=2; ambient_pulse_speed_turn(1);
    for(i=0;i<4;i++)ambient_latch_toggle(&trk[0],48+i*4,100);
    ambient_latch_toggle(&trk[1],60,100);ambient_latch_toggle(p,72,100);seq_start();
    int pressure_ok=1;
    for(i=0;i<100;i++) {
        if (!(i%10)) {
            ambient_latch_clear(&trk[0]);
            for(uint32_t j=0;j<4;j++)ambient_latch_toggle(&trk[0],48+j*4+(i/10)%12,100);
            ambient_latch_toggle(&trk[1],60+(i/10)%12,100);
            if(!ambient_layers[1].count)ambient_latch_toggle(&trk[1],60+(i/10)%12,100);
        }
        ambient_latch_toggle(p,72+i%12,100);pulse_blocks(200);
        pressure_ok &= ambient_gated_notes(&trk[0])==4&&ambient_gated_notes(&trk[1])==1;
        ambient_latch_clear(p);
    }
    bad+=ambient_check("dense arp plus chord/register changes preserve five sustained voices",pressure_ok);
    ambient_audio_reset(); ambient_layers[2].density=100;ambient_density_apply(p);ambient_latch_toggle(p,0,100); seq_start();pulse_blocks(1);
    bad+=ambient_check("lowest MIDI pitch is a tracked generated gate",ambient_layers[2].arp_gate&&gate_note(p,0));
    seq_stop();pulse_blocks(1);
    bad+=ambient_check("STOP releases MIDI pitch zero",!ambient_layers[2].arp_gate&&!gate_note(p,0));
    ambient_audio_reset();usb.config=1;mo_r=mo_w=0;song.sel=2;ambient_tap_key(7);
    uint32_t on=0,off=0;while(mo_r!=mo_w) {uint32_t e=midi_out_q[mo_r++%MQ];on+=((e>>8)&0xf0)==0x90;}
    panic_req|=4;events_block(CTL);
    while(mo_r!=mo_w) {uint32_t e=midi_out_q[mo_r++%MQ];off+=((e>>8)&0xf0)==0x80;}
    bad+=ambient_check("physical hold and panic balance outgoing MIDI",on==1&&off==1);
    ambient_audio_reset();song.sel=2;
    btn_down(B_HOME);frame();int16_t level=p->p[P_LEVEL];turn(EN_K3,1);turn(EN_K4,1);turn(EN_SELECT,1);
    bad+=ambient_check("MIX knobs own levels and HOME SELECT changes only PULSE speed",p->p[P_LEVEL]==level+1&&ambient_layers[2].density==50&&ambient_layers[2].space==50&&ambient_layers[2].speed==3&&p->p[P_ARATE]==2);
    btn_up(B_HOME);frame();turn(EN_ALGO,1);
    bad+=ambient_check("ambient HOME reaches layer four",song.sel==3);
    /* All HOME screens now use the curated sound list; sound changes start silent. */
    ambient_audio_reset();usb.config=1;mo_r=mo_w=0;song.sel=2;
    ambient_tap_key(7);ambient_tap_key(11);seq_start();pulse_blocks(1);ambient_muted=4;
    int16_t before_global[G_COUNT];memcpy(before_global,song.g,sizeof before_global);
    int16_t before_level = p->p[P_LEVEL];
    turn(EN_PRESET,1);pulse_blocks(100);
    bad+=ambient_check("HOME sound selection keeps PULSE macros/mix/mute and clears old input",
        ui.home&&pulse_active(p)&&ambient_layers[2].timbre==1&&!ambient_layers[2].count&&!p->nheld&&
        p->p[P_LEVEL]==before_level&&ambient_muted==4&&song.playing&&!memcmp(before_global,song.g,sizeof before_global));
    ambient_tap_key(7);seq_stop();
    bad+=ambient_check("new PULSE input remains queued after STOP",ambient_layers[2].count==1&&!ambient_layers[2].arp_gate);
    btn_down(B_HOME);frame();uint32_t timbre=ambient_layers[2].timbre;turn(EN_PRESET,1);btn_up(B_HOME);frame();
    bad+=ambient_check("MIX preset turns do not change sound",ambient_layers[2].timbre==timbre);
    press(B_EDIT);turn(EN_PRESET,1);events_block(CTL);
    bad+=ambient_check("full UI retains the general preset library",!ui.home&&!pulse_active(p)&&!ambient_layers[2].count);
    /* External-clock arp advancement uses supplied clock samples, not wall samples. */
    ambient_audio_reset();song.g[G_CLOCK]=1;ambient_latch_toggle(p,60,100);seq_start();
    arp_step(p,0,0);uint32_t idx=p->arp_idx;
    for(i=0;i<100;i++)arp_step(p,0,CTL);
    bad+=ambient_check("external clock pause does not generate extra plucks",p->arp_idx==idx);
    arp_step(p,div_samples(p->p[P_ARATE]),CTL);
    bad+=ambient_check("one clock division produces one new pluck",p->arp_idx==idx+1);
    seq_stop();bad+=ambient_check("external STOP retains input and releases gate",ambient_layers[2].count==1&&!ambient_layers[2].arp_gate);
    /* Candidate renders use existing presets, with a conservative common release. */
    for(i=0;i<3;i++) {
        static const uint8_t engines[]={2,0,9},presets[]={4,8,2};
        static const char *names[]={"phase", "analog", "phys"};
        ambient_audio_reset();set_engine_of(p,engines[i]);apply_preset_to(p,presets[i]);events_block(CTL);
        p->engine=p->eng_req;p->p[P_AMODE]=0;p->p[P_REL]=42;p->p[P_LEVEL]=72;
        p->p[P_DLY]=p->p[P_REV]=0;
        char path[120];snprintf(path,sizeof path,"build/pulse_demo/candidate-%s.wav",names[i]);
        FILE *wav=fopen(path,"wb");if(wav)wav_hdr(wav,(6u*FS/CTL)*CTL);
        uint64_t start=now_ns();
        for(uint32_t f=0;f+CTL<=6u*FS;f+=CTL) {
            if(f%(FS/CTL*CTL)<CTL)trk_note_on(p,60+(f/FS%3)*7,100);
            if(f%(FS/CTL*CTL)>=FS/8 && f%(FS/CTL*CTL)<FS/8+CTL)trk_all_off(p);
            int32_t out[CTL*2];mix_block(out,CTL);
            if(wav)for(uint32_t j=0;j<CTL;j++)wav_put(wav,out[2*j],out[2*j+1]);
        }
        if(wav)fclose(wav);printf("candidate %s host cost %.3f of realtime\n",names[i],(now_ns()-start)/6e9);
    }
    bad+=pulse_timing();bad+=pulse_random_notes();bad+=pulse_mute();bad+=pulse_variation();bad+=pulse_audio();
    printf("pulse: %s\n",bad?"FAILED":"all passed");return bad?1:0;
}
