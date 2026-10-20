/* SPDX-License-Identifier: GPL-3.0-only */
/* Listening scene: existing AmbWave scheduler, mixer, shared voices and FX. */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "../../tests/ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    ambient_audio_reset();ambient_patch_apply(&trk[0],AS_TIDE,9);ambient_patch_apply(&trk[2],AS_PULSE,3);
    ambient_mode_set(&trk[2],AM_SCATTER);ambient_layers[2].speed=0;
    ambient_density_turn(&trk[2],-35);ambient_mode_apply(&trk[2]);
    trk[0].p[P_LEVEL]=90;trk[2].p[P_LEVEL]=75;
    ambient_space_turn(&trk[2],-100);ambient_muted=0;
    int32_t out[CTL*2];for(unsigned j=0;j<100;j++)mix_block(out,CTL);
    rng_state=0x1234567u;mod_seed=0x2545F491u;
    ambient_latch_toggle(&trk[0],48,80);ambient_latch_toggle(&trk[0],55,80);
    ambient_latch_toggle(&trk[2],72,80);ambient_latch_toggle(&trk[2],79,80);seq_start();
    FILE *f=fopen(argv[1],"wb");if(!f)return 2;wav_hdr(f,90*FS);
    int32_t peak=0;double energy=0;unsigned stage=0;
    for(unsigned pos=0;pos<90*FS;pos+=CTL) {
        unsigned sec=pos/FS;
        if(sec>=5&&sec<25)ambient_density_turn(&trk[0],(int)((sec-5)*4)-ambient_layers[0].density);
        if(sec>=25&&stage==0){ambient_density_turn(&trk[0],80-ambient_layers[0].density);stage++;}
        if(sec>=35&&stage==1){ambient_shape_turn(&trk[0],15);stage++;}
        if(sec>=45&&stage==2){ambient_motion_turn(&trk[0],50);stage++;}
        if(sec>=55&&stage==3){ambient_space_turn(&trk[0],50);ambient_space_turn(&trk[2],50);stage++;}
        if(sec>=70&&stage==4){seq_stop();ambient_latch_clear(&trk[0]);ambient_latch_clear(&trk[2]);stage++;}
        unsigned n=90*FS-pos<CTL?90*FS-pos:CTL;mix_block(out,n);
        for(unsigned j=0;j<n;j++){wav_put(f,out[2*j],out[2*j+1]);for(unsigned c=0;c<2;c++){int a=abs(out[2*j+c]);if(a>peak)peak=a;energy+=(double)a*a;}}
    }
    fclose(f);printf("90s BOWL STUDY + sparse GLASS BELL: peak %d RMS %.2f\n",peak,sqrt(energy/(180.*FS)));
    return peak>=32767||!peak;
}
