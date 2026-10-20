/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* web/emu/felucca_web.c built natively (cc, as the host tests), playing emu_test.mjs's heavy song with the same
 * gestures and the same render calls: its samples, float32 L R interleaved, to OUT. emu_test.mjs compares the
 * WebAssembly build's against them (the browser plays what the host build plays).
 *   cc -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src -o build/emu/native_check web/emu/native_check.c -lm
 *   build/emu/native_check OUT.f32 */
#include "felucca_web.c"

static FILE *out;
static int rec;
static void render(double ms)                     /* as emu_test.mjs: 128-frame calls, the last one shorter */
{
    uint32_t n = (uint32_t)(ms * 44.1 + 0.5), k, i;
    for (k = 0; k < n; k += 128u) {
        uint32_t m = n - k < 128u ? n - k : 128u;
        web_render(m);
        for (i = 0; rec && i < m; i++) {
            float s[2] = {out_l[i], out_r[i]};
            fwrite(s, sizeof s, 1, out);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2 || !(out = fopen(argv[1], "wb")))
        return 2;
    web_nor_erase();
    web_boot();
#if AMBIENT_POC
    if (argc > 2 && !strcmp(argv[2], "drone")) {
        render(1000); rec = 1;
        web_buttons(1u << B_HOME); render(40);
        web_enc(EN_PRESET,-1); render(40); web_buttons(0); render(40);
        web_keys((1u<<7)|(1u<<11)); render(60); web_keys(0); render(60000);
        web_enc(EN_ALGO,1); render(40);
        web_keys(1u<<14); render(60); web_keys(0); render(1000);
        web_enc(EN_ALGO,-1); render(40);
        web_keys((1u<<9)|(1u<<13)); render(60); web_keys(0); render(1000);
        for(uint32_t knob=EN_K1;knob<=EN_K4;knob++) { web_enc(knob,20); render(40); }
        render(3000); fclose(out); return 0;
    }
    if (argc > 2 && (!strcmp(argv[2], "factory-tide") || !strcmp(argv[2], "factory-pulse"))) {
        uint32_t pulse = !strcmp(argv[2], "factory-pulse"), count = pulse ? 6 : 10;
        render(1000); rec = 1;
        if (pulse) {
            for (uint32_t k = 0; k < 2; k++) { web_enc(EN_ALGO, 1); render(40); }
            web_buttons(1u << B_PLAY); render(60); web_buttons(0); render(60);
        }
        for (uint32_t patch = 1; patch < count; patch++) {
            web_enc(EN_PRESET, 1); render(40);
            if (patch < 3) continue;
            web_keys((1u << 7) | (1u << 11)); render(60); web_keys(0); render(2000);
        }
        fclose(out); return 0;
    }
    if (argc > 2 && !strcmp(argv[2], "pads")) {
        render(1000);rec=1;
        for(uint32_t pad=0;pad<2;pad++) {
            for (uint32_t turn = 0; turn < (pad ? 1u : 2u); turn++) {
                web_enc(EN_PRESET,pad ? -1 : 1);render(40);
            }
            web_keys((1u<<7)|(1u<<11)|(1u<<14));render(60);web_keys(0);render(4000);
            web_keys((1u<<7)|(1u<<11)|(1u<<14));render(60);web_keys(0);render(4000);
        }
        fclose(out);return 0;
    }
    if (argc > 2) {
        render(1000); rec = 1;
        for (uint32_t stage = 0; stage < 5; stage++) {
            web_test_texture_stage(stage); render(1000);
        }
        fclose(out); return 0;
    }
#endif
    render(500);
    web_test_heavy();
    web_buttons(1u << B_PLAY);
    render(60);
    web_buttons(0);
    render(60);
    rec = 1;
    render(2000);
    fclose(out);
    return 0;
}
