/* SPDX-License-Identifier: GPL-3.0-only */
/* Repeatable five-minute Drone solo or four-track listening scene. */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "../tests/ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
#include <time.h>
int main(int argc,char **argv)
{
    if(argc!=4) { fprintf(stderr,"usage: drone-audition OUT.wav MOTION solo|scene\n"); return 2; }
    uint32_t motion=(uint32_t)atoi(argv[2]), scene=!strcmp(argv[3],"scene");
    if(motion>100 || (!scene && strcmp(argv[3],"solo")))return 2;
    ambient_audio_reset(); rng_state=0x1234567u; mod_seed=0x2545F491u;
    ambient_patch_apply(&trk[0],AS_DRONE,0); ambient_motion_turn(&trk[0],(int32_t)motion-25);
    if(scene) {
        ambient_patch_apply(&trk[1],AS_TIDE,0); ambient_patch_apply(&trk[2],AS_TEXTURE,0);
        ambient_patch_apply(&trk[3],AS_PULSE,2); ambient_mode_set(&trk[3],AM_SCATTER);
        ambient_density_turn(&trk[3],-35); ambient_layers[3].speed=0; ambient_mode_apply(&trk[3]);
        trk[0].p[P_LEVEL]=72;trk[1].p[P_LEVEL]=68;trk[2].p[P_LEVEL]=35;trk[3].p[P_LEVEL]=60;
    }
    int32_t out[CTL*2]; for(uint32_t j=0;j<100;j++)mix_block(out,CTL);
    ambient_drone_add(&trk[0],36,100); ambient_drone_add(&trk[0],43,100);
    if(scene) {
        ambient_latch_toggle(&trk[1],60,90);ambient_latch_toggle(&trk[1],67,90);
        ambient_latch_toggle(&trk[2],48,80);ambient_latch_toggle(&trk[3],72,90);seq_start();
    }
    FILE *f=fopen(argv[1],"wb");if(!f)return 2;
    uint32_t frames=300u*FS;wav_hdr(f,frames);double energy=0,sum=0;int32_t peak=0;uint32_t clipped=0;
    clock_t start=clock();
    for(uint32_t pos=0;pos<frames;pos+=CTL) {
        uint32_t n=frames-pos<CTL?frames-pos:CTL;mix_block(out,n);
        int16_t pcm[CTL*2];
        for(uint32_t j=0;j<2*n;j++) {
            pcm[j]=(int16_t)out[j];int32_t a=out[j]<0?-out[j]:out[j];if(a>peak)peak=a;
            clipped+=a>=32767;sum+=out[j];energy+=(double)out[j]*out[j];
        }
        fwrite(pcm,sizeof(int16_t),n*2,f);
    }
    fclose(f);
    printf("Drone %s Motion=%u: 300s peak=%d RMS=%.2f DC=%.2f clipped=%u host/realtime=%.6f gates=%u\n",
        scene?"scene":"solo",motion,peak,sqrt(energy/(frames*2.)),sum/(frames*2.),clipped,
        (double)(clock()-start)/CLOCKS_PER_SEC/300.,ambient_gated_notes(&trk[0]));
    return clipped || !peak || ambient_gated_notes(&trk[0])!=2;
}
