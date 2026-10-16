/* SPDX-License-Identifier: GPL-3.0-only */
/* Listening-only patch experiment using the existing FM6 engine unchanged.
 * Compile this same harness against archived 86d2167 for the original take.
 * Nothing here installs a patch in firmware or writes device/user storage. */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "../tests/ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

static void grounded_patch(track_t *t, int more_mids)
{
    uint8_t patch[FP_SIZE+1]; fm6_unpack(FM6_INIT,patch);
    patch[FP_ALG]=4; patch[FP_FB]=0; patch[FP_OKS]=1;
    patch[FP_LPMD]=patch[FP_LAMD]=0;
    for(unsigned op=0;op<6;op++) {
        uint8_t *p=patch+op*FP_OP;
        p[FP_OL]=0; p[FP_MODE]=0; p[FP_FC]=1; p[FP_FF]=0; p[FP_DET]=7;
        p[FP_KVS]=p[FP_RS]=p[FP_AMS]=p[FP_LD]=p[FP_RD]=0;
        p[FP_R1]=50; p[FP_R1+1]=99; p[FP_R1+2]=99; p[FP_R1+3]=45;
        p[FP_L1]=p[FP_L1+1]=p[FP_L1+2]=99; p[FP_L1+3]=0;
    }
    /* Algorithm 5: one 1:1 FM pair and a quiet, unmodulated 2nd harmonic.
     * No extra fundamental carriers, spread, feedback or pitch modulation. */
    patch[5*FP_OP+FP_OL]=99;                 /* OP1 carrier, stable sustain */
    patch[4*FP_OP+FP_OL]=66;                 /* OP2 modulator, ~0.115 turns at EG level 99 */
    patch[4*FP_OP+FP_R1]=42;                 /* harmonic bloom after onset */
    /* Keep each audition reproducible: 0 original, 1 subtle, 2 strong mids. */
    patch[3*FP_OP+FP_OL]=more_mids==2?87:more_mids?78:75;
    patch[3*FP_OP+FP_FC]=2;
    memcpy(patch+FP_NAME,"TIDE BODY ",10);
    fm6_set_patch(0,patch); fm6_adopt(0);
    for(unsigned p=P_E0;p<P_E7;p++)t->p[p]=0;
    t->p[P_LRATE]=9; t->p[P_LWAVE]=5;
    t->p[P_LD_FLT]=10;                        /* existing DRIFT -> FM index */
    t->p[P_ED_FLT]=t->p[P_LD_PIT]=t->p[P_LD_SHP]=t->p[P_LD_AMP]=0;
}

int main(int argc,char **argv)
{
    if(argc<3)return 2;
    int candidate=atoi(argv[1]), single=argc>3;
    ambient_audio_reset(); track_t *t=&trk[0];
    if(candidate) {ambient_sound_apply(t,AS_TIDE,2);grounded_patch(t,candidate>1?candidate-1:0);}
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
           candidate==3?"FM strong mids":candidate>1?"FM mids":candidate?"FM candidate":"Original",single?"single":"Dm7",rms,dc,peak,tail_rms,active,cost);
    bad|=peak>=32767||fabs(dc)>100||rms<200||tail_rms>1||active||cost>0.5;
    return ambient_check("dry FM patch audition: zero FX return, health, release and host cost",!bad);
}
