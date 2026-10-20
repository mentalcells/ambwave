/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

static void pad_blocks(uint32_t n) { int32_t out[CTL*2];while(n--)mix_block(out,CTL); }

typedef struct { double colour, correlation, movement, tail; } pad_macro_measure_t;
static int16_t pad_low_audio[8u*FS];
/* Measure actual held-note audio after moving a knob, rather than just checking
 * its parameter values. Difference energy is a brightness proxy, normalized
 * correlation measures changes beyond gain, and brightness variation measures
 * tonal movement. Felucca's shared effect return is mono. */
static pad_macro_measure_t pad_macro_render(uint32_t timbre,uint32_t macro,int high)
{
    ambient_audio_reset();track_t *t=&trk[0];ambient_patch_apply(t,AS_TIDE,timbre);pad_blocks(100);
    void (*turn_macro[])(track_t *,int32_t)={ambient_shape_turn,ambient_motion_turn,ambient_density_turn,ambient_space_turn};
    turn_macro[macro](t,-100);
    ambient_latch_toggle(t,48,100);ambient_latch_toggle(t,55,100);ambient_latch_toggle(t,64,100);
    double e[8]={0},diff[8]={0},cross=0,reference=0,tail=0;uint32_t ntail=0;int32_t previous=0;
    for(uint32_t f=0;f<20u*FS;f+=CTL) {
        if(high&&f/CTL==2u*FS/CTL)turn_macro[macro](t,100);
        if(f/CTL==12u*FS/CTL)ambient_latch_clear(t);
        int32_t out[CTL*2];mix_block(out,CTL);
        for(uint32_t i=0;i<CTL;i++) {
            uint32_t frame=f+i;
            int32_t mono=(out[i*2]+out[i*2+1])/2,delta=mono-previous;previous=mono;
            if(frame>=4u*FS&&frame<12u*FS) {
                uint32_t bin=frame/FS-4;
                e[bin]+=(double)mono*mono;diff[bin]+=(double)delta*delta;
                uint32_t at=frame-4u*FS;
                if(!high)pad_low_audio[at]=(int16_t)mono;
                cross+=(double)mono*pad_low_audio[at];reference+=(double)pad_low_audio[at]*pad_low_audio[at];
            }
            if(frame>=14u*FS&&frame<16u*FS) {tail+=(double)mono*mono;ntail++;}
        }
    }
    pad_macro_measure_t m={0};double energy=0,totaldiff=0,lo=1e9,hi=0;
    for(uint32_t b=0;b<8;b++) {
        double c=diff[b]/(e[b]+1);if(c<lo)lo=c;if(c>hi)hi=c;
        energy+=e[b];totaldiff+=diff[b];
    }
    m.colour=totaldiff/(energy+1);m.correlation=cross/sqrt((energy+1)*(reference+1));
    m.movement=(hi-lo)/(m.colour+1e-9);m.tail=sqrt(tail/ntail);
    return m;
}

static int check_macro_response(void)
{
    int bad=0;
    for(uint32_t timbre=1;timbre<3;timbre++)for(uint32_t macro=0;macro<4;macro++) {
        pad_macro_measure_t low=pad_macro_render(timbre,macro,0),high=pad_macro_render(timbre,macro,1);
        printf("macro %s %u: colour %.5f/%.5f correlation %.3f movement %.3f/%.3f tail %.2f/%.2f\n",ambient_patch_find(AS_TIDE, timbre)->name,macro,low.colour,high.colour,high.correlation,low.movement,high.movement,low.tail,high.tail);
        int audible=macro==0?high.colour>low.colour*1.5:
            macro==1?high.movement>low.movement+0.2:
            macro==2?fabs(high.correlation)<0.90:high.tail>low.tail+1;
        bad+=ambient_check("moving each pad macro changes held-note tone, movement, thickness or space",audible);
    }
    return bad;
}

static int render_pad(uint32_t timbre, uint32_t wet, const char *directory)
{
    ambient_audio_reset();track_t *t=&trk[0];
    ambient_patch_apply(t,AS_TIDE,timbre);pad_blocks(100);
    /* Identical sends isolate the source/envelope comparison from SPACE curves. */
    t->p[P_DLY]=wet?16:0;t->p[P_REV]=wet?42:0;t->p[P_CHOR]=0;
    ambient_latch_toggle(t,48,100);ambient_latch_toggle(t,55,100);ambient_latch_toggle(t,64,100);
    FILE *wav=0;
    if(directory) {
        char path[512];snprintf(path,sizeof path,"%s/%s_%s.wav",directory,ambient_patch_find(AS_TIDE, timbre)->name,wet?"space":"dry");
        wav=fopen(path,"wb");if(!wav)return 1;wav_hdr(wav,18u*FS);
    }
    double energy=0,sum=0,tail=0;uint32_t count=0,ntail=0;int32_t peak=0;
    for(uint32_t f=0;f<18u*FS;f+=CTL) {
        if(f/CTL==6u*FS/CTL)ambient_latch_clear(t);
        int32_t out[CTL*2];mix_block(out,CTL);
        for(uint32_t i=0;i<CTL*2;i++) {
            int32_t a=out[i]<0?-out[i]:out[i];if(a>peak)peak=a;
            if(f>=3u*FS&&f<6u*FS){energy+=(double)out[i]*out[i];sum+=out[i];count++;}
            if(f>=16u*FS){tail+=(double)out[i]*out[i];ntail++;}
        }
        if(wav)for(uint32_t i=0;i<CTL&&f+i<18u*FS;i++)wav_put(wav,out[i*2],out[i*2+1]);
    }
    if(wav)fclose(wav);
    uint32_t active=0;for(uint32_t v=0;v<NVOICE;v++)active+=t->v[v].active;
    printf("pad %s %s: RMS %.1f peak %d DC %.1f tail %.1f voices %u\n",ambient_patch_find(AS_TIDE, timbre)->name,wet?"space":"dry",sqrt(energy/count),peak,sum/count,sqrt(tail/ntail),active);
    return ambient_check("pad chord has body/headroom and decays without hanging voices",sqrt(energy/count)>300&&peak<32767&&fabs(sum/count)<300&&sqrt(tail/ntail)<100&&!active);
}

int main(int argc,char **argv)
{
    int bad=check_macro_response();
    for(uint32_t timbre=0;timbre<3;timbre++)for(uint32_t wet=0;wet<2;wet++)bad+=render_pad(timbre,wet,argc>1?argv[1]:0);
    for(uint32_t timbre=1;timbre<3;timbre++)for(uint32_t corner=0;corner<16;corner++) {
        ambient_audio_reset();track_t *t=&trk[0];ambient_patch_apply(t,AS_TIDE,timbre);pad_blocks(100);
        ambient_shape_turn(t,corner&1?100:-100);ambient_motion_turn(t,corner&2?100:-100);
        ambient_density_turn(t,corner&4?100:-100);ambient_space_turn(t,corner&8?100:-100);
        ambient_latch_toggle(t,48,100);ambient_latch_toggle(t,55,100);ambient_latch_toggle(t,60,100);ambient_latch_toggle(t,64,100);
        double energy=0,sum=0;uint32_t count=0;int32_t peak=0;
        uint64_t start=now_ns();
        for(uint32_t f=0;f<4u*FS;f+=CTL) {
            int32_t out[CTL*2];mix_block(out,CTL);
            if(f>FS)for(uint32_t i=0;i<CTL*2;i++) {
                int32_t a=out[i]<0?-out[i]:out[i];if(a>peak)peak=a;
                energy+=(double)out[i]*out[i];sum+=out[i];count++;
            }
        }
        bad+=ambient_check("warm-pad four-note macro corners are audible, bounded and within host budget",sqrt(energy/count)>30&&peak<32767&&fabs(sum/count)<300&&(now_ns()-start)<2e9&&ambient_gated_notes(t)==4);
    }
    /* Real preset wheel: new sounds are available on all tracks and every mode. */
    for(uint32_t k=0;k<NTRK;k++)for(uint32_t mode=0;mode<3;mode++)for(uint32_t timbre=1;timbre<3;timbre++) {
        ambient_audio_reset();song.sel=k;track_t *t=&trk[k];
        ambient_patch_apply(t,AS_TIDE,0);ambient_mode_set(t,mode);
        t->step[0].n=1;t->step[0].note[0]=67;t->step[0].time=ST_NOTE;t->p[P_LEVEL]=63;
        int16_t globals[G_COUNT];memcpy(globals,song.g,sizeof globals);
        turn(EN_PRESET,timbre);pad_blocks(100);
        bad+=ambient_check("pad selection preserves HOME, track mode/pattern/level and shared FX",ui.home&&ambient_home_active(t)&&ambient_layer(t)->patch==timbre&&ambient_layer(t)->mode==mode&&t->step[0].note[0]==67&&t->p[P_LEVEL]==63&&!memcmp(globals,song.g,sizeof globals));
        ambient_latch_toggle(t,60,100);
        ambient_mode_set(t,(mode+1)%3);pad_blocks(100);
        bad+=ambient_check("pad mode change releases local input without changing sound identity",ambient_home_active(t)&&ambient_layer(t)->patch==timbre&&!ambient_layer(t)->count&&!ambient_gated_notes(t));
    }
    return bad?1:0;
}
