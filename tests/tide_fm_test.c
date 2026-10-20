/* SPDX-License-Identifier: GPL-3.0-only */
/* Integrated TIDE: dry routing, sustained chord, headroom and release.
 * Defaults reproduce the user-reviewed FM audition without overriding DSP. */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

int main(int argc,char **argv)
{
    if(argc<3)return 2;
    int candidate=atoi(argv[1]), single=argc>3;
    for(unsigned timbre=0;timbre<3;timbre++) {
        ambient_audio_reset(); track_t *route=&trk[0];
        ambient_patch_apply(route,AS_TIDE,timbre);
        ambient_space_turn(route,-100); ambient_density_turn(route,100);
        if(route->p[P_CHOR] || route->p[P_DLY] || route->p[P_REV])return 1;
        ambient_space_turn(route,100); ambient_density_turn(route,-100);
        ambient_space_turn(route,-100);
        if(route->p[P_CHOR] || route->p[P_DLY] || route->p[P_REV])return 1;
    }
    ambient_audio_reset(); track_t *t=&trk[0];
    if(candidate) ambient_density_turn(t, 50); /* Exercise maximum harmonic weight and release. */
    ambient_space_turn(t,-100);
    t->p[P_DIST]=t->p[P_CHOR]=t->p[P_DLY]=t->p[P_REV]=0;
    if(single) {
        if(!candidate){ambient_density_turn(t,-100);t->p[P_LD_FLT]=t->p[P_ED_FLT]=t->p[P_E3]=0;}
    }
    int32_t out[CTL*2]; for(unsigned b=0;b<100;b++)mix_block(out,CTL);
    const uint8_t notes[]={50,57,65,72};      /* same four-note Dm7 in both takes */
    for(unsigned k=0;k<(single?1:4);k++)ambient_latch_toggle(t,notes[k],100);
    FILE *wav=fopen(argv[2],"wb");if(!wav)return 2; wav_hdr(wav,30u*FS);
    double energy=0,sum=0,tail=0;unsigned count=0;int peak=0,bad=0;
    uint64_t start=now_ns();
    for(unsigned f=0;f<30u*FS;f+=CTL) {
        if(f/CTL==20u*FS/CTL)ambient_latch_clear(t);
        mix_block(out,CTL);
        for(unsigned i=0;i<CTL&&f+i<30u*FS;i++) {
            unsigned at=f+i;
            for(unsigned ch=0;ch<2;ch++){
                int x=out[i*2+ch],a=abs(x);if(a>peak)peak=a;
                if(at>=8u*FS&&at<20u*FS){energy+=(double)x*x;sum+=x;count++;}
                if(at>=29u*FS)tail+=(double)x*x;
            }
            bad|=send_c[i]||send_d[i]||send_r[i]||wet[i];
            bad|=out[i*2]!=out[i*2+1];         /* no stereo placement/widening advantage */
            wav_put(wav,out[i*2],out[i*2+1]);
        }
    }
    fclose(wav);unsigned active=0;for(unsigned v=0;v<NVOICE;v++)active+=t->v[v].active;
    double rms=sqrt(energy/count),dc=sum/count,tail_rms=sqrt(tail/(2*FS));
    double cost=(now_ns()-start)/30e9;
    printf("%s %s: RMS %.1f DC %.1f peak %d tail %.2f voices %u cost %.4f real time\n",
           candidate?"Integrated dense Tide":"Integrated default Tide",single?"single":"Dm7",rms,dc,peak,tail_rms,active,cost);
    bad|=peak>=32767||fabs(dc)>100||rms<200||tail_rms>1||active||cost>0.5;
    /* Exercise Density after note-on: patch OL edits alone would not change
     * the held envelope. Inspect the rendered operator gain, not the mapping. */
    ambient_audio_reset(); t=&trk[0];
    ambient_space_turn(t,-100); ambient_motion_turn(t,-100);
    ambient_latch_toggle(t,57,100);
    for(unsigned b=0;b<10u*FS/CTL;b++)mix_block(out,CTL);
    fm6_note_t *n=NULL;
    for(unsigned v=0;v<NVOICE;v++)if(t->v[v].active)n=fm6_note_of(t,&t->v[v]);
    if(!n)bad=1;
    else {
        int32_t pitch=n->op[3].freq;
        ambient_density_turn(t,-100);
        for(unsigned b=0;b<4;b++)mix_block(out,CTL);
        int32_t low=n->op[3].gain_out;
        ambient_density_turn(t,100);
        for(unsigned b=0;b<4;b++)mix_block(out,CTL);
        bad |= n->op[3].gain_out < low*15 || n->op[3].freq != pitch;
        bad |= t->p[P_CHOR] || t->p[P_DLY] || t->p[P_REV] || t->p[P_E6];
    }
    return ambient_check("dry FM patch audition: zero FX return, health, release and host cost",!bad);
}
