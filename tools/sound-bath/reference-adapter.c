/* SPDX-License-Identifier: GPL-3.0-only
 * Sound Bath audition adapter. Synthesis, voices and effects are unmodified Felucca.
 * Reuses the upstream host's initialization and exact factory loader. */
#define main felucca_host_main
#include "../../tests/hostsim.c"
#undef main
#undef __attribute__
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define API EMSCRIPTEN_KEEPALIVE
#else
#define API
#endif
static const unsigned engine_ids[] = {ENGI_FM6, ENGI_FM6, ENGI_PHYS, ENGI_PHYS, ENGI_PHYS, ENGI_PHYS};
static const unsigned preset_ids[] = {1, 4, 0, 3, 7, 3};
static float output[2048];
static unsigned selected, ready;
static int brightness_target, excitation_target, wet_target;
static unsigned smooth_clock;
API int sbb_init(unsigned patch, unsigned wet)
{
    if (ready || patch >= 6) return 0; /* reset by creating a fresh host/wasm instance */
    selected = patch;
    host_tracks_init();
    host_preset(&trk[0], engine_ids[patch], preset_ids[patch]);
    if (patch == 5) { /* explicit variant, never changes PHYS_PRESETS */
        trk[0].p[P_ATK] = 0;
        trk[0].p[P_REL] = 100;
        trk[0].p[P_E6] = 0;
    }
    song.g[G_RTYPE] = 2; /* controlled HALL; dry source is otherwise identical */
    song.g[G_RSIZE] = 70;
    song.g[G_RDAMP] = 80;
    trk[0].p[P_CHOR] = 0;
    trk[0].p[P_DLY] = 0;
    wet_target = wet ? 32 : 0;
    trk[0].p[P_REV] = wet_target;
    brightness_target = trk[0].p[P_E2];
    excitation_target = trk[0].p[P_E6];
    ready = 1;
    return 1;
}
API void sbb_note(unsigned note, unsigned down, unsigned velocity)
{
    if (!ready || note < 24 || note > 96) return;
    if (down) trk_note_on(&trk[0], note, clamp(velocity, 1, 127));
    else trk_note_off(&trk[0], note);
}
API void sbb_release(void)
{
    for (unsigned note = 24; note <= 96; note++) sbb_note(note, 0, 80);
}
API void sbb_wet(unsigned wet) { wet_target = wet ? 32 : 0; }
API void sbb_brightness(int offset)
{
    int base = ENGINES[engine_ids[selected]]->presets[preset_ids[selected]].e[2];
    brightness_target = clamp(base + clamp(offset, -20, 20), selected < 2 ? -64 : 0, selected < 2 ? 63 : 127);
}
API void sbb_excitation(unsigned value)
{
    if (selected == 2 || selected == 3 || selected == 5) excitation_target = clamp(value, 0, 100);
}
static void approach(int16_t *p, int target) { *p += (*p < target) - (*p > target); }
API float *sbb_output(void) { return output; }
API void sbb_render(unsigned frames)
{
    if (frames > 1024) frames = 1024;
    for (unsigned k = 0; k < frames; k += CTL) {
        int32_t block[2 * CTL];
        unsigned n = frames - k < CTL ? frames - k : CTL;
        /* Existing parameter controls, slewed at ~1 ms; no additional synthesis. */
        if (++smooth_clock >= 2) {
            smooth_clock = 0;
            approach(&trk[0].p[P_E2], brightness_target);
            approach(&trk[0].p[P_E6], excitation_target);
            approach(&trk[0].p[P_REV], wet_target);
        }
        mix_block(block, n);
        for (unsigned i = 0; i < 2*n; i++) output[2*k+i] = block[i] / 32768.0f;
    }
}
API unsigned sbb_active(void)
{
    unsigned n = 0;
    for (unsigned i = 0; i < NVOICE; i++) n += trk[0].v[i].active != 0;
    return n;
}
API unsigned sbb_capacity(void) { return ENGINES[engine_ids[selected]]->poly; }
API unsigned sbb_engine_bytes(void) { return selected < 2 ? sizeof fm6_note[0] : sizeof phys_slot[0]; }
API unsigned sbb_host_bytes(void) { return sizeof(trk) + sizeof(song); }
API unsigned sbb_patch_value(unsigned i) { return i < 8 ? trk[0].p[P_E0+i] : 0; }
#ifndef __EMSCRIPTEN__
int main(int argc, char **argv)
{
    if (argc != 5) { fprintf(stderr,"usage: render PATCH WET SCENARIO OUTPUT.wav\n"); return 2; }
    unsigned patch = atoi(argv[1]), wet = atoi(argv[2]);
    const char *scenario = argv[3];
    if (!sbb_init(patch, wet)) return 2;
    unsigned seconds = !strcmp(scenario,"bowl") ? 90 : !strcmp(scenario,"sustain") ? 76 : !strcmp(scenario,"strike") ? 114 : 40;
    unsigned frames = seconds * FS, peak = 0, maxvoices = 0;
    double energy = 0, late = 0;
    FILE *f = fopen(argv[4],"wb");
    if (!f) return 2;
    wav_hdr(f,frames);
    uint64_t start = now_ns();
    unsigned offmask = 0;
    for (unsigned pos = 0; pos < frames; pos += 128) {
        double t = (double)pos / FS;
        if (!strcmp(scenario,"strike")) {
            /* C3/C4/C5: 8s held + 30s release, 38s each. */
            for (unsigned j=0;j<3;j++) {
                if (pos >= j*38u*FS && !(offmask & (1u<<(2*j)))) { sbb_note(48+12*j,1,80); offmask |= 1u<<(2*j); }
                if (pos >= (j*38u+8)*FS && !(offmask & (2u<<(2*j)))) { sbb_note(48+12*j,0,80); offmask |= 2u<<(2*j); }
            }
        } else if (!strcmp(scenario,"sustain")) {
            if (!pos) sbb_note(48,1,80);
            if (t>=60 && !(offmask&1)) { sbb_release(); offmask|=1; }
        } else if (!strcmp(scenario,"repeat")) {
            unsigned beat = (unsigned)t;
            if (beat<8 && !(offmask & (1u<<beat))) { sbb_note(60,1,80); offmask|=1u<<beat; }
            if (t>=8 && !(offmask&256)) { sbb_release(); offmask|=256; }
        } else if (!strcmp(scenario,"interaction")) {
            /* Three separate phrases: fifth, octave, semitone/three notes. */
            const unsigned notes[3][3]={{48,55,0},{48,60,0},{60,61,67}};
            unsigned phrase=(unsigned)t/12;
            if (phrase<3 && !(offmask&(1u<<(phrase*2)))) {
                for(unsigned j=0;j<3;j++) if(notes[phrase][j]) sbb_note(notes[phrase][j],1,80);
                offmask|=1u<<(phrase*2);
            }
            if (phrase<3 && t-phrase*12>=7 && !(offmask&(2u<<(phrase*2)))) { sbb_release(); offmask|=2u<<(phrase*2); }
        } else if (!strcmp(scenario,"control")) {
            if (!pos) sbb_note(48,1,80);
            if (t<30) {
                sbb_brightness((int)(20*sin(t*0.25)));
                if (patch>=2) sbb_excitation((unsigned)(50+50*sin(t*0.2-1.5707963)));
            } else if (!(offmask&1)) { sbb_release(); offmask|=1; }
        } else if (!strcmp(scenario,"bowl")) {
            if (!pos) sbb_note(48,1,80);
            if (t>=5 && t<25) sbb_excitation((unsigned)((t-5)*4));
            if (t>=25 && t<70) sbb_excitation(80);
            if (t>=70 && !(offmask&1)) { sbb_release(); offmask|=1; }
        } else if (!strcmp(scenario,"pressure")) {
            if (!pos) for(unsigned j=0;j<10;j++) sbb_note(48+j,1,80);
            if (t>=5 && !(offmask&1)) { sbb_release(); offmask|=1; }
        } else return 2;
        unsigned n=frames-pos<128?frames-pos:128;
        sbb_render(n);
        if(sbb_active()>maxvoices) maxvoices=sbb_active();
        for(unsigned i=0;i<n;i++) {
            int32_t l=(int32_t)(output[2*i]*32768),r=(int32_t)(output[2*i+1]*32768);
            unsigned a=abs(l)>abs(r)?abs(l):abs(r);
            if(a>peak)peak=a;
            energy+=(double)l*l+(double)r*r;
            if(t>=50 && t<60)late+=(double)l*l+(double)r*r;
            wav_put(f,l,r);
        }
    }
    fclose(f);
    double elapsed=(now_ns()-start)/1e9;
    printf("{\"patch\":%u,\"wet\":%u,\"scenario\":\"%s\",\"seconds\":%u,\"render_seconds\":%.6f,\"realtime_percent\":%.3f,\"peak\":%u,\"rms\":%.3f,\"late_rms\":%.3f,\"max_voices\":%u,\"end_voices\":%u,\"voice_cap\":%u,\"engine_bytes\":%u,\"tracks_song_bytes\":%u}\n",patch,wet,scenario,seconds,elapsed,elapsed/seconds*100,peak,sqrt(energy/(2*frames)),sqrt(late/(2*10*FS)),maxvoices,sbb_active(),sbb_capacity(),sbb_engine_bytes(),sbb_host_bytes());
    return peak>=32767 ? 1 : 0;
}
#endif
