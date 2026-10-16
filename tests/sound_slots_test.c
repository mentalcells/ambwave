/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
#include <time.h>

static void slots_blocks(uint32_t n)
{
    int32_t out[CTL * 2];
    while (n--) mix_block(out, CTL);
}
static int check_switches(void)
{
    int bad = 0;
    uint32_t k, choice, i;
    static const uint8_t sounds[] = {AS_TEXTURE, AS_CLOUD, AS_PULSE, AS_PULSE, AS_PULSE, AS_TIDE, AS_TIDE, AS_TIDE};
    static const uint8_t timbres[] = {0, 0, 0, 1, 2, 1, 2, 0};
    for (k = 0; k < NTRK; k++) for (choice = 0; choice < 8; choice++) {
        ambient_audio_reset(); usb.config = 1; mo_r = mo_w = 0;
        song.sel = k;
        ambient_sound_apply(&trk[k], AS_TIDE, 0); slots_blocks(100);
        for (i = 0; i < choice; i++) { turn(EN_PRESET, 1); slots_blocks(100); }
        /* Exercise the actual key/MIDI path before each sound change. */
        ambient_tap_key(7); slots_blocks(10);
        uint32_t ons = 0, offs = 0;
        while (mo_r != mo_w) { uint32_t e = midi_out_q[mo_r++ % MQ]; ons += ((e >> 8) & 0xf0) == 0x90; }
        ambient_muted |= 1u << k;
        trk[k].p[P_LEVEL] = 47;
        trk[k].step[0].note[0] = 67;
        int16_t global[G_COUNT]; memcpy(global, song.g, sizeof global);
        ambient_layer_t neighbors[NTRK]; memcpy(neighbors, ambient_layers, sizeof neighbors);
        int16_t params[NTRK][P_COUNT];
        for (i = 0; i < NTRK; i++) memcpy(params[i], trk[i].p, sizeof params[i]);
        turn(EN_PRESET, 1); slots_blocks(100);
        while (mo_r != mo_w) { uint32_t e = midi_out_q[mo_r++ % MQ]; offs += ((e >> 8) & 0xf0) == 0x80; }
        int independent = !memcmp(global, song.g, sizeof global);
        for (i = 0; i < NTRK; i++) if (i != k)
            independent &= !memcmp(&neighbors[i], &ambient_layers[i], sizeof neighbors[i]) &&
                !memcmp(params[i], trk[i].p, sizeof params[i]);
        bad += ambient_check("every slot selects every sound without losing HOME/mix/mute/steps",
            ui.home && ambient_home_active(&trk[k]) && ambient_layers[k].sound == sounds[choice] && ambient_layers[k].timbre == timbres[choice] &&
            !ambient_layers[k].count && !trk[k].nheld && !ambient_gated_notes(&trk[k]) &&
            trk[k].p[P_LEVEL] == 47 && (ambient_muted & (1u << k)) && trk[k].step[0].note[0] == 67);
        bad += ambient_check("sound change balances physical MIDI and preserves other tracks/FX",
            ons == 1 && offs == 1 && independent);
        /* All macros are usable on the moved sound and leave neighbors alone. */
        ambient_shape_turn(&trk[k], 100); ambient_motion_turn(&trk[k], 100);
        ambient_density_turn(&trk[k], 100); ambient_space_turn(&trk[k], 100);
        for (i = 0; i < NTRK; i++) if (i != k)
            independent &= !memcmp(&neighbors[i], &ambient_layers[i], sizeof neighbors[i]) &&
                !memcmp(params[i], trk[i].p, sizeof params[i]);
        independent &= !memcmp(global, song.g, sizeof global);
        bad += ambient_check("macros on moved sounds are independent and leave shared FX fixed", independent);
    }
    return bad;
}
static int check_duplicate_sounds(void)
{
    int bad = 0;
    ambient_audio_reset();
    for (uint32_t k = 0; k < NTRK; k++) { ambient_sound_apply(&trk[k], AS_PULSE, k % 3); ambient_mode_set(&trk[k], AM_SCATTER); }
    slots_blocks(100);
    for (uint32_t k = 0; k < NTRK; k++) ambient_latch_toggle(&trk[k], k ? 60 + k : 0, 100);
    seq_start(); slots_blocks(20);
    int independent = 1;
    for (uint32_t k = 0; k < NTRK; k++) independent &= ambient_layers[k].count == 1 && trk[k].nheld == 1;
    seq_stop(); slots_blocks(1);
    for (uint32_t k = 0; k < NTRK; k++) independent &= !ambient_layers[k].arp_gate && !ambient_gated_notes(&trk[k]);
    bad += ambient_check("four PULSE slots own their inputs/gates, including pitch zero; STOP releases all", independent);
    ambient_latch_clear(&trk[0]);
    bad += ambient_check("clearing one PULSE slot preserves the other three inputs",
        !ambient_layers[0].count && ambient_layers[1].count && ambient_layers[2].count && ambient_layers[3].count);
    ambient_audio_reset();
    for (uint32_t k = 0; k < NTRK; k++) ambient_sound_apply(&trk[k], AS_CLOUD, 0);
    slots_blocks(100);
    for (uint32_t k = 0; k < NTRK; k++) ambient_latch_toggle(&trk[k], 60 + k, 100);
    ambient_latch_clear(&trk[1]);
    bad += ambient_check("duplicate CLOUD slots retain independent one-note holds",
        ambient_layers[0].count && !ambient_layers[1].count && ambient_layers[2].count && ambient_layers[3].count);
    return bad;
}
static int check_four_layer_audio(void)
{
    int bad = 0, healthy = 1, headroom = 1;
    double worst = 0;
    for (uint32_t timbre = 0; timbre < 3; timbre++) for (uint32_t corner = 0; corner < 16; corner++) {
        ambient_audio_reset();
        ambient_sound_apply(&trk[0], AS_TIDE, timbre); slots_blocks(100);
        for (uint32_t k = 0; k < NTRK; k++) {
            ambient_shape_turn(&trk[k], corner & 1 ? 100 : -100);
            ambient_motion_turn(&trk[k], corner & 2 ? 100 : -100);
            ambient_density_turn(&trk[k], corner & 4 ? 100 : -100);
            ambient_space_turn(&trk[k], corner & 8 ? 100 : -100);
        }
        ambient_latch_toggle(&trk[0], 48, 100); ambient_latch_toggle(&trk[0], 55, 100);
        ambient_latch_toggle(&trk[0], 60, 100); ambient_latch_toggle(&trk[0], 64, 100);
        for (uint32_t k = 1; k < NTRK; k++) ambient_latch_toggle(&trk[k], 60 + k * 5, 100);
        seq_start();
        double energy = 0, sum = 0; uint32_t count = 0; int32_t peak = 0;
        uint64_t start = now_ns();
        for (uint32_t f = 0; f < 4u * FS; f += CTL) {
            int32_t out[CTL * 2]; mix_block(out, CTL);
            if (f > FS) {
                uint32_t voices = 0;
                for (uint32_t k = 0; k < NTRK; k++) for (uint32_t v = 0; v < NVOICE; v++) voices += trk[k].v[v].active;
                headroom &= voices <= NVOICE && ambient_gated_notes(&trk[0]) == 4 &&
                    ambient_gated_notes(&trk[1]) == 1 && ambient_gated_notes(&trk[3]) == 1;
                for (uint32_t i = 0; i < CTL * 2; i++) {
                    int32_t a = out[i] < 0 ? -out[i] : out[i]; if (a > peak) peak = a;
                    energy += (double)out[i] * out[i]; sum += out[i]; count++;
                }
            }
        }
        double cost = (now_ns() - start) / (4e9);
        if (cost > worst) worst = cost;
        healthy &= peak < 32767 && sqrt(energy / count) > 30 && fabs(sum / count) < 300;
        seq_stop(); for (uint32_t k = 0; k < NTRK; k++) ambient_latch_clear(&trk[k]);
    }
    printf("four-layer host render worst %.3f of real time\n", worst);
    bad += ambient_check("four-layer macro corners stay audible, bounded and free of excessive DC", healthy);
    bad += ambient_check("default four-layer voice pressure preserves sustained gates within budget", headroom);
    bad += ambient_check("four-layer host render cost remains below half real time", worst < 0.5);
    return bad;
}
int main(void)
{
    int bad = check_switches() + check_duplicate_sounds() + check_four_layer_audio();
    return bad ? 1 : 0;
}
