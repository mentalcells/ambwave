/* SPDX-License-Identifier: GPL-3.0-only */
/* Offline P0b patch corners, using unchanged firmware DSP. */
#define TIDE_BREATH_EXPERIMENT 0
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
#include "../tools/tide-voicing-patches.h"

int main(void)
{
    const unsigned notes[]={36,50,72,96};
    unsigned failures=0,cases=0;
    for(unsigned variant=0;variant<4;variant++) {
        char choice[2]={(char)('0'+variant),0};setenv("TIDE_VOICING",choice,1);
        for(unsigned ni=0;ni<4;ni++)for(unsigned shape=0;shape<=100;shape+=50)
        for(unsigned density=0;density<=100;density+=50) {
            ambient_audio_reset();track_t *t=&trk[0];
            ambient_motion_turn(t,-100);ambient_space_turn(t,-100);
            ambient_shape_turn(t,(int)shape-50);ambient_density_turn(t,(int)density-50);
            int16_t params[P_COUNT];memcpy(params,t->p,sizeof params);
            uint8_t original[FP_SIZE+1];memcpy(original,fm6_patch[0],sizeof original);
            tide_voicing_setup(t);
            if(variant==3)for(unsigned p=0;p<sizeof original;p++)
                if(p!=FP_OP+FP_OL && p!=FP_OP+FP_DET && original[p]!=fm6_patch[0][p])failures++;
            for(unsigned p=0;p<P_COUNT;p++)if(p!=P_E7&&params[p]!=t->p[p])failures++;
            unsigned active=0;
            for(unsigned op=0;op<6;op++)active+=fm6_patch[0][op*FP_OP+FP_OL]!=0;
            if(active!=(variant>=2?4u:3u))failures++;
            int32_t out[CTL*2];for(unsigned b=0;b<100;b++)mix_block(out,CTL);
            ambient_latch_toggle(t,notes[ni],100);
            int peak=0,tail=0,bad=0;double dc=0,energy=0;unsigned n=0;
            for(unsigned frame=0;frame<15u*FS;frame+=CTL) {
                if(frame/CTL==3u*FS/CTL)ambient_latch_clear(t);
                mix_block(out,CTL);
                if(voices_busy()>8)bad=1;
                for(unsigned i=0;i<CTL;i++) {
                    int x=out[2*i],a=abs(x);if(a>peak)peak=a;
                    bad|=send_c[i]||send_d[i]||send_r[i]||wet[i]||x!=out[2*i+1];
                    if(frame+i>=FS&&frame+i<3u*FS){dc+=x;energy+=(double)x*x;n++;}
                    if(frame+i>=14u*FS&&a>tail)tail=a;
                }
            }
            dc/=n;
            bad|=peak>=32767||peak<100||fabs(dc)>100||tail>2||voices_busy()||energy<1;
            if(bad)fprintf(stderr,"P0b FAIL variant%u note%u shape%u density%u peak%d DC%.2f tail%d\n",variant,notes[ni],shape,density,peak,dc,tail);
            failures+=bad;cases++;
        }
    }
    printf("P0b: %u patch/pitch/macro corners, %u failures; dry mono, release, parameter integrity, active operators and shared voice limit.\n",cases,failures);
    return failures?1:0;
}
