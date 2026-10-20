/* SPDX-License-Identifier: GPL-3.0-only */
/* Same source harness compiled against pristine and experimental firmware.
 * Every take runs in a fresh process. No device, preset or project writes. */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "../tests/ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
#ifdef __APPLE__
#include <libproc.h>
#include <sys/resource.h>
static uint64_t instructions(void)
{
    struct rusage_info_v4 r;
    return proc_pid_rusage(getpid(),RUSAGE_INFO_V4,(rusage_info_t *)&r) ? 0 : r.ri_instructions;
}
#else
static uint64_t instructions(void) { return 0; }
#endif

#ifdef TIDE_RENDER_SETUP_HEADER
#include TIDE_RENDER_SETUP_HEADER
#endif

int main(int argc,char **argv)
{
    if(argc!=6) {fprintf(stderr,"usage: render OUT.wav single|chord|high|mix|silk|warm|scatter HOLD_SECONDS MOTION GESTURES\n");return 2;}
    unsigned hold=(unsigned)atoi(argv[3]), motion=(unsigned)atoi(argv[4]), gestures=(unsigned)atoi(argv[5]);
    if(hold<15||hold>600||motion>100)return 2;
    ambient_audio_reset(); rng_state=0x1234567u; mod_seed=0x2545F491u;
    track_t *t=&trk[0];
    if(!strcmp(argv[2],"silk"))ambient_patch_apply(t,AS_TIDE,2);
    if(!strcmp(argv[2],"warm"))ambient_patch_apply(t,AS_TIDE,1);
    if(!strcmp(argv[2],"scatter"))ambient_mode_set(t,AM_SCATTER);
    for(unsigned k=0;k<NTRK;k++)ambient_space_turn(&trk[k],-100);
    ambient_motion_turn(t,(int)motion-ambient_layer(t)->motion);
#ifdef TIDE_RENDER_SETUP_HEADER
    tide_voicing_setup(t);
#endif
    int32_t out[CTL*2]; for(unsigned b=0;b<100;b++)mix_block(out,CTL);
    const uint8_t low[]={50,57,65,72}, high[]={62,69,77,84};
    const uint8_t *notes=!strcmp(argv[2],"high")?high:low;
    unsigned count=!strcmp(argv[2],"single")?1:4;
    if(strcmp(argv[2],"all-tide"))for(unsigned k=0;k<count;k++)ambient_latch_toggle(t,notes[k],100);
    else {
        for(unsigned tr=0;tr<NTRK;tr++) {
            ambient_patch_apply(&trk[tr],AS_TIDE,0);
            ambient_mode_set(&trk[tr],AM_HOLD);ambient_space_turn(&trk[tr],-100);
            ambient_motion_turn(&trk[tr],(int)motion-ambient_layer(&trk[tr])->motion);
#ifdef TIDE_RENDER_SETUP_HEADER
            tide_voicing_setup(&trk[tr]);
#endif
        }
        /* Drain the real preset-change panic/engine-switch requests before
         * starting the stress chord, as in the other audition scenarios. */
        for(unsigned b=0;b<100;b++)mix_block(out,CTL);
        for(unsigned tr=0;tr<NTRK;tr++) {
            ambient_latch_toggle(&trk[tr],50+tr*5,100);ambient_latch_toggle(&trk[tr],57+tr*5,100);
        }
    }
    int mix=!strcmp(argv[2],"mix");
    if(mix) {ambient_latch_toggle(&trk[1],60,100);ambient_latch_toggle(&trk[3],67,100);
        for(unsigned k=0;k<3;k++)ambient_latch_toggle(&trk[2],72+k*4,100);seq_start();}
    if(!strcmp(argv[2],"scatter"))seq_start();
    if(!strcmp(argv[1],"bench")) {
        for(unsigned b=0;b<2u*FS/CTL;b++)mix_block(out,CTL);
        unsigned frames=hold*FS/CTL*CTL;
        uint64_t ns=now_ns(), cpu=instructions();
        for(unsigned b=0;b<frames/CTL;b++)mix_block(out,CTL);
        uint64_t end=instructions();ns=now_ns()-ns;
        printf("{\"instructions_per_frame\":%.5f,\"ns_per_frame\":%.5f,\"voices\":%u}\n",
            end>cpu?(double)(end-cpu)/frames:0,(double)ns/frames,voices_busy());
        return 0;
    }
    FILE *f=fopen(argv[1],"wb");if(!f)return 2;
    unsigned total=(hold+12)*FS; wav_hdr(f,total);
    uint64_t cpu0=instructions(), ns0=now_ns(), render_ns=0;
    int peak=0,bad=0,tailpeak=0,maxvoices=0,prepeak=0;
    double sum=0,energy=0,limmax=0; unsigned n=0;
    for(unsigned frame=0;frame<total;frame+=CTL) {
        if(frame/CTL==hold*FS/CTL) {seq_stop();for(unsigned k=0;k<NTRK;k++){ambient_latch_clear(&trk[k]);trk_all_off(&trk[k]);}}
        if(gestures && frame/CTL==10u*FS/CTL) {ambient_shape_turn(t,40);ambient_density_turn(t,45);}
        if(gestures && frame/CTL==20u*FS/CTL) {ambient_motion_turn(t,100);ambient_shape_turn(t,-80);}
        if(gestures && frame/CTL==30u*FS/CTL) {ambient_motion_turn(t,-100);ambient_density_turn(t,-90);}
        uint64_t t0=now_ns();mix_block(out,CTL);render_ns+=now_ns()-t0;
        unsigned busy=voices_busy();if((int)busy>maxvoices)maxvoices=(int)busy;
        if(busy>NVOICE)bad=1;
        if(lim_env>limmax)limmax=lim_env;
        for(unsigned i=0;i<CTL&&frame+i<total;i++) {
            bad |= send_c[i]||send_d[i]||send_r[i]||wet[i];
            if(!mix)bad |= out[2*i]!=out[2*i+1];
            for(unsigned ch=0;ch<2;ch++) {
                int x=out[2*i+ch],a=abs(x);if(a>peak)peak=a;
                int pre=abs(ch?mix_r[i]:mix_l[i]);if(pre>prepeak)prepeak=pre;
                if(frame+i>=8u*FS&&frame+i<hold*FS){sum+=x;energy+=(double)x*x;n++;}
                if(frame+i>=(hold+11)*FS&&a>tailpeak)tailpeak=a;
            }
            wav_put(f,out[2*i],out[2*i+1]);
        }
    }
    uint64_t cpu1=instructions(); fclose(f);
    double rms=sqrt(energy/n), dc=sum/n;
    bad |= peak>=32767||fabs(dc)>100||rms<100||tailpeak>2||voices_busy();
    printf("{\"rms\":%.5f,\"dc\":%.5f,\"peak\":%d,\"tail_peak\":%d,\"max_voices\":%d,\"remaining_voices\":%u,\"pre_master_peak\":%d,\"limiter_max_db\":%.5f,\"render_ns_per_frame\":%.5f,\"instructions_per_frame_with_io\":%.5f,\"wall_seconds\":%.5f,\"ok\":%s}\n",
        rms,dc,peak,tailpeak,maxvoices,voices_busy(),prepeak,20*log10(limmax/LIM_T),
        (double)render_ns/total,cpu1>cpu0?(double)(cpu1-cpu0)/total:0,(now_ns()-ns0)/1e9,bad?"false":"true");
    return bad;
}
