/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_POC 1
#define UI_RENDER_MAIN felucca_render_main
#include "ui_render.c"
int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "build/texture_ui";
    char path[600], name[80];
    uint32_t p, z, st, focus, mix, timbre;
    snprintf(path, sizeof path, "%s/report.txt", out);
    rep = fopen(path, "w");
    if (!rep) return 1;
    for (p = 0; p < NPALETTES; p++) for (z = 0; z < 2; z++)
        for (st = 0; st < 2; st++) for (focus = 0; focus < 3; focus++) for (mix = 0; mix < 2; mix++) for (timbre = 0; timbre < 3; timbre++) {
            state(); ambient_boot_apply(); ambient_layers[2].patch = timbre; ambient_pulse_timbre_apply(&trk[2]); pal(p);
            ui_style = (uint8_t)st; style_apply();
            if (z) ui_prefs |= PREF_LARGE; else ui_prefs &= ~PREF_LARGE;
            song.sel = focus; ambient_mix = mix;
            if (mix) ui.layer = LAYER_FX; /* MIX also wins over an open quick-layer map. */
            ambient_layers[0].count = 4; ambient_layers[1].count = 1; ambient_layers[2].count = 4; song.playing = 1;
            trk[0].p[P_LEVEL] = trk[1].p[P_LEVEL] = 127;
            snprintf(name, sizeof name, "%s_%s_%s_%s_%s_%u", UI_PALETTES[p].name,
                z ? "LARGE" : "NORMAL", st ? "LINE" : "FLAT", focus == 2 ? "PULSE" : focus ? "TEXTURE" : "TIDE", mix ? "MIX" : "HOME", timbre);
            cur_name = name;
            draw(-1); lint(); mono_check();
            if (p == UI_GREY_INDEX) write_ppm(out, "GREY", name);
        }
    fclose(rep);
    printf("texture UI: all themes/styles/card sizes, findings %u, ink %u, neutrality %u, alignment %u\n", nfind, nink, mono_bad, al_fail);
    return nfind || nink || mono_bad || al_fail ? 1 : 0;
}
