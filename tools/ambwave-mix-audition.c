/* SPDX-License-Identifier: GPL-3.0-only */
/* Listening aid, not a replacement for perceived-loudness acceptance.
 * Render both mixes in the same process with identical notes and seeds. */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "../tests/ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: ambwave-mix-audition OUTPUT_DIR\n"); return 1; }
    const char *names[] = {"tide", "pulse", "three-note-layered", "four-note-layered", "all-layers"};
    for (unsigned candidate = 0; candidate < 2; candidate++) for (unsigned scenario = 0; scenario < 5; scenario++) {
        ambient_audio_reset(); rng_state = 0x1234567u; mod_seed = 0x2545F491u;
        if (!candidate) {
            /* Preserve the original comparison after adopting the accepted
             * mix. Candidate now uses the actual firmware boot defaults. */
            const uint8_t levels[] = {100, 80, 72, 64};
            for (unsigned k = 0; k < NTRK; k++) {
                trk[k].p[P_LEVEL] = levels[k]; ambient_level[k] = LEVEL_Q12[levels[k]];
            }
        }
        if (scenario != 1) {
            ambient_latch_toggle(&trk[0], 50, 100);
            ambient_latch_toggle(&trk[0], 57, 100);
            ambient_latch_toggle(&trk[0], 65, 100);
            if (scenario == 3) ambient_latch_toggle(&trk[0], 72, 100);
        }
        if (scenario != 0) {
            ambient_latch_toggle(&trk[2], 72, 100);
            ambient_latch_toggle(&trk[2], 76, 100);
            ambient_latch_toggle(&trk[2], 79, 100);
            seq_start();
        }
        if (scenario == 4) {
            ambient_latch_toggle(&trk[1], 60, 100); ambient_latch_toggle(&trk[3], 67, 100);
        }
        char path[512]; snprintf(path, sizeof path, "%s/%s-%s.wav", argv[1], candidate ? "candidate" : "current", names[scenario]);
        FILE *wav = fopen(path, "wbx"); if (!wav) { perror(path); return 1; }
        const unsigned frames = (12u * FS / CTL) * CTL;
        wav_hdr(wav, frames); double energy = 0; unsigned n = 0; int peak = 0;
        for (unsigned frame = 0; frame < frames; frame += CTL) {
            int32_t out[CTL * 2]; mix_block(out, CTL);
            for (unsigned i = 0; i < CTL; i++) {
                wav_put(wav, out[2*i], out[2*i+1]);
                for (unsigned ch = 0; ch < 2; ch++) {
                    int v = out[2*i+ch]; if (abs(v) > peak) peak = abs(v);
                    if (frame >= 2u * FS) { energy += (double)v*v; n++; }
                }
            }
        }
        fclose(wav);
        printf("%s: RMS %.1f, peak %d (%.2f dBFS)\n", path, sqrt(energy/n), peak, 20*log10((double)peak/32768));
    }
    return 0;
}
