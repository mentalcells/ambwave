/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

static void settle(void)
{
    int32_t out[CTL * 2];
    for (int i = 0; i < 100; i++) mix_block(out, CTL);
}

int main(void)
{
    int bad = 0;
    uint32_t corner, f, i;
    ui_power_on(); song.sel = 1;
    int16_t level = trk[1].p[P_LEVEL];
    int16_t globals[G_COUNT];
    memcpy(globals, song.g, sizeof globals);
    ambient_tap_key(7);
    presets_turn(1); settle();
    bad += ambient_check("HOME selects CLOUD silently, preserving mix and shared FX",
        cloud_active(&trk[1]) && !ambient_layers[1].count && trk[1].p[P_LEVEL] == level &&
        !memcmp(globals, song.g, sizeof globals) && str_eq(ambient_scene_name(), "CLOUD"));
    ambient_tap_key(7);
    bad += ambient_check("CLOUD uses layer-2 toggle hold", ambient_layers[1].count && gate_note(&trk[1], 60));
    ambient_tap_key(7);
    bad += ambient_check("CLOUD releases its held note", !ambient_layers[1].count);
    for (corner = 0; corner < 16; corner++) {
        double energy = 0, solo = 0;
        int peak = 0;
        ambient_audio_reset(); song.sel = 1; presets_turn(1); settle();
        ambient_shape_turn(&trk[1], corner & 1 ? 100 : -100);
        ambient_motion_turn(&trk[1], corner & 2 ? 100 : -100);
        ambient_density_turn(&trk[1], corner & 4 ? 100 : -100);
        ambient_space_turn(&trk[1], corner & 8 ? 100 : -100);
        ambient_tap_key(7);
        for (f = 0; f < 2u * FS; f += CTL) {
            if (f < FS && f + CTL >= FS) {
                trk_note_on(&trk[0], 48, 100); trk_note_on(&trk[0], 55, 100);
                trk_note_on(&trk[0], 60, 100); trk_note_on(&trk[0], 64, 100);
            }
            int32_t out[CTL * 2]; mix_block(out, CTL);
            for (i = 0; i < CTL * 2; i++) {
                int a = out[i] < 0 ? -out[i] : out[i];
                if (a > peak) peak = a;
                energy += (double)out[i] * out[i];
                if (f < FS) solo += (double)out[i] * out[i];
            }
        }
        bad += ambient_check("layered CLOUD corner stays audible and bounded", solo > 0 && energy > 0 && peak <= 32767);
        ambient_latch_clear(&trk[1]); trk_all_off(&trk[0]);
        presets_turn(-1);
        bad += ambient_check("HOME returns to curated TEXTURE", texture_active(&trk[1]) && !(ambient_layers[1].sound == AS_CLOUD) && !ambient_layers[1].count);
    }
    return bad ? 1 : 0;
}
