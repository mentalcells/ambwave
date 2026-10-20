/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
static int reference_source;

/* Also compiled against the pre-experiment sources to produce an honest A/B.
 * No external FX, no other layers, and freshly cleared shared effect buffers. */
static int render_dry(const char *directory, int isolated, int bright)
{
    ambient_audio_reset();
    track_t *t = &trk[0];
    ambient_space_turn(t, -100);
    /* Explicit bypass also makes the old-source reference genuinely dry. */
    t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    if (isolated) {
        ambient_density_turn(t, -100);
        ambient_motion_turn(t, isolated == 2 ? -100 : 100);
        t->p[P_LD_FLT] = t->p[P_ED_FLT] = t->p[P_E3] = 0;
    }
    if (bright) ambient_shape_turn(t, 100);
    int32_t out[CTL * 2];
    for (unsigned b = 0; b < 100; b++) mix_block(out, CTL);
    ambient_latch_toggle(t, bright ? 76 : 48, 100);
    if (!isolated && !bright) {
        ambient_latch_toggle(t, 55, 100);
        ambient_latch_toggle(t, 64, 100);
    }
    char path[512]; FILE *wav = NULL;
    if (directory) {
        snprintf(path, sizeof path, "%s/%s.wav", directory,
                 isolated == 2 ? "single-source-still" : isolated ? "single-source" : bright ? "bright-register" : "default-chord");
        wav = fopen(path, "wb"); if (!wav) return 1;
        wav_hdr(wav, 30u * FS);
    }
    int bad = 0, peak = 0; double sum = 0, energy = 0; unsigned count = 0;
    /* Goertzel bins measure individual partials, rather than accepting another
     * broad brightness change as evidence for the new source hypothesis. */
    double coefficient[3], partials[12][3][2] = {{{0}}};
    for (unsigned h = 0; h < 3; h++)
        coefficient[h] = 2 * cos(2 * 3.141592653589793 * (h+1) * 440 * pow(2, (48-69)/12.0) / FS);
    for (unsigned f = 0; f < 30u * FS; f += CTL) {
        if (f / CTL == 20u * FS / CTL) ambient_latch_clear(t);
        mix_block(out, CTL);
        for (unsigned i = 0; i < CTL && f + i < 30u * FS; i++) {
            int x = out[i * 2], a = abs(x); if (a > peak) peak = a;
            unsigned frame = f + i;
            if (frame >= 8u * FS && frame < 20u * FS) {
                unsigned bin = frame / FS - 8;
                sum += x; energy += (double)x*x; count++;
                if (isolated) for (unsigned h = 0; h < 3; h++) {
                    double *state = partials[bin][h];
                    double next = x + coefficient[h] * state[0] - state[1];
                    state[1] = state[0]; state[0] = next;
                }
            }
            if (wav) wav_put(wav, x, out[i*2+1]);
            /* Check real bus inputs and return throughout sustain and release. */
            bad |= send_c[i] || send_d[i] || send_r[i] || wet[i];
        }
    }
    if (wav) fclose(wav);
    if (isolated && !reference_source && t->eng_req == 0) {
        double lo = 1e30, hi = 0, fundamental_lo = 1e30, fundamental_hi = 0;
        for (unsigned b = 0; b < 12; b++) {
            double power[3];
            for (unsigned h = 0; h < 3; h++) {
                double *state = partials[b][h];
                power[h] = state[0]*state[0] + state[1]*state[1] - coefficient[h]*state[0]*state[1];
            }
            double ratio = power[1] / (power[2] + 1);
            if (ratio < lo) lo = ratio;
            if (ratio > hi) hi = ratio;
            if (power[0] < fundamental_lo) fundamental_lo = power[0];
            if (power[0] > fundamental_hi) fundamental_hi = power[0];
        }
        printf("dry source motion %s: H2/H3 power ratio %.2f..%.2f, fundamental amplitude spread %.3f\n",
               isolated == 2 ? "zero" : "full", lo, hi, sqrt(fundamental_hi/fundamental_lo));
        bad |= ambient_check("source has a stable fundamental at 8..20s without filter drift or detune", fundamental_hi < fundamental_lo * 1.21);
        bad |= ambient_check("MOTION controls independent harmonic evolution throughout sustain",
                             isolated == 2 ? hi < lo * 1.10 : hi > lo * 4);
    }
    unsigned active = 0; for (unsigned v = 0; v < NVOICE; v++) active += t->v[v].active;
    printf("dry %d/%d: RMS %.1f DC %.1f peak %d\n", isolated, bright, sqrt(energy/count), sum/count, peak);
    return ambient_check("isolated dry render: zero FX buses/return, bounded signal and released voices",
                         !bad && peak < 32767 && sqrt(energy/count) > 200 && fabs(sum/count) < 100 && !active);
}

int main(int argc, char **argv)
{
    int bad = 0;
    reference_source = argc > 2;
    /* All Tide variants: either macro order must keep SPACE=0 fully dry,
     * then restore the existing chorus mapping when SPACE is raised. */
    if (argc < 3) for (unsigned timbre = 0; timbre < 3; timbre++) {
        ambient_audio_reset(); track_t *t = &trk[0]; ambient_patch_apply(t, AS_TIDE, timbre);
        ambient_space_turn(t, -100); ambient_density_turn(t, 100);
        bad += ambient_check("SPACE=0 disables every Tide spatial send after DENSITY edits",
                             !t->p[P_CHOR] && !t->p[P_DLY] && !t->p[P_REV]);
        ambient_space_turn(t, 50);
        bad += ambient_check("SPACE restores chorus only for WARM and SILK", t->p[P_CHOR] == (timbre ? 96 : 0));
    }
    for (int c = 0; c < 4; c++) bad += render_dry(argc > 1 ? argv[1] : NULL, c == 3 ? 2 : c == 1, c == 2);
    return bad ? 1 : 0;
}
