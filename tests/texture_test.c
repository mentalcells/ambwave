/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
#include <time.h>

static void texture_clear(void)
{
    btn_down(B_FX); frame(); press(B_OCTDN); btn_up(B_FX); frame();
    events_block(CTL);
}
static void texture_render(uint32_t blocks)
{
    int32_t out[CTL * 2];
    while (blocks--) mix_block(out, CTL);
}
/* Render all 16 texture corners with TIDE at both extremes, then default demos.
 * The timer is a host regression guard; target loop budgets remain authoritative. */
static int texture_audio_checks(void)
{
    uint32_t corner, f, i;
    int bad = 0, healthy = 1;
    int32_t maxpeak = 0;
    double maxdc = 0, maxlate = 0, worstcost = 0;
    for (corner = 0; corner < 67; corner++) {
        int32_t out[2 * CTL], peak = 0;
        double energy = 0, sum = 0, late = 0;
        uint32_t count = 0, latecount = 0;
        char path[100]; FILE *wav = 0;
        ambient_audio_reset();
        if (corner < 64) {
            uint32_t a = corner & 15u;
            if (corner & 32) trk[0].p[P_LEVEL] = trk[1].p[P_LEVEL] = 127;
            ambient_shape_turn(&trk[1], a & 1 ? 100 : -100);
            ambient_layers[1].motion = a & 2 ? 100 : 0; ambient_motion_apply(&trk[1]);
            ambient_layers[1].density = a & 4 ? 100 : 0; ambient_density_apply(&trk[1]);
            ambient_layers[1].space = a & 8 ? 100 : 0; ambient_space_apply(&trk[1]);
            ambient_shape_turn(&trk[0], corner & 16 ? 127 : -127);
            ambient_layers[0].motion = ambient_layers[0].density = ambient_layers[0].space = corner & 16 ? 100 : 0;
            ambient_motion_apply(&trk[0]); ambient_density_apply(&trk[0]); ambient_space_apply(&trk[0]);
        } else {
            static const char *const names[] = {"tide-default", "texture-default", "layers-default"};
            snprintf(path, sizeof path, "build/texture_demo/%s.wav", names[corner - 64]);
            wav = fopen(path, "wb"); if (wav) wav_hdr(wav, (10u * FS / CTL) * CTL);
        }
        if (corner != 65) {
            trk_note_on(&trk[0], 48, 100); trk_note_on(&trk[0], 55, 100);
            trk_note_on(&trk[0], 60, 100); trk_note_on(&trk[0], 64, 100);
        }
        if (corner != 64) trk_note_on(&trk[1], 60, 100);
        uint64_t start = now_ns();
        for (f = 0; f + CTL <= 10u * FS; f += CTL) {
            if (f < 4u * FS && f + CTL >= 4u * FS) { trk_all_off(&trk[0]); trk_all_off(&trk[1]); }
            mix_block(out, CTL);
            for (i = 0; i < CTL; i++) {
                int32_t l = out[2 * i], r = out[2 * i + 1];
                int32_t a = l < 0 ? -l : l, b = r < 0 ? -r : r;
                if (a > peak) peak = a; if (b > peak) peak = b;
                if (f >= 2u * FS && f < 4u * FS) {
                    energy += (double)l*l + (double)r*r; sum += l + r; count += 2;
                }
                if (f >= 9u * FS) { late += (double)l*l + (double)r*r; latecount += 2; }
                if (wav) wav_put(wav, l, r);
            }
        }
        double cost = (now_ns() - start) / 1e10, rms = sqrt(energy / count), dc = fabs(sum / count), tail = sqrt(late / latecount);
        if (wav) { fclose(wav); printf("texture render: %s RMS %.1f peak %d DC %.1f late %.1f cost %.3f\n", path, rms, peak, dc, tail, cost); }
        if (cost > worstcost) worstcost = cost;
        if (peak > maxpeak) maxpeak = peak;
        if (dc > maxdc) maxdc = dc; if (tail > maxlate) maxlate = tail;
        healthy &= peak < 32767 && dc < 300 && tail < 300 && rms > 30;
    }
    printf("texture corners: max peak %d, DC %.1f, late RMS %.1f, host cost %.3f of realtime\n", maxpeak, maxdc, maxlate, worstcost);
    bad += ambient_check("all texture macro corners at default/max mix with TIDE extremes stay bounded and decay", healthy);
    bad += ambient_check("layer render host CPU cost remains below half realtime", worstcost < 0.5);
    return bad;
}
int main(void)
{
    int bad = 0, isolated, balanced, stable = 1;
    uint32_t i, j, mo0;
    int16_t params[P_COUNT], global[G_COUNT];
    track_t *t = &trk[1];
    ambient_audio_reset();
    usb.config = 1; mo_w = mo_r = 0;
    bad += ambient_check("TEXTURE boots silent, unmuted ANLG with fixed drift/crush off",
        texture_active(t) && t->engine == 11u && !ambient_layers[1].count && !t->p[P_MUTE] &&
        !t->p[P_E0] && !t->p[P_E6] && !t->p[P_E7] && ambient_layers[1].shape == 35 &&
        ambient_layers[1].motion == 25 && ambient_layers[1].density == 25 && ambient_layers[1].space == 50);
    ambient_tap_key(7); ambient_tap_key(9); ambient_tap_key(11); ambient_tap_key(12);
    turn(EN_ALGO, 1);
    ambient_tap_key(7);
    bad += ambient_check("focus changes preserve four TIDE notes and TEXTURE holds independently",
        song.sel == 1 && ambient_layers[1].count && gate_note(t, 60) && ambient_gated_notes(&trk[0]) == 4);
    texture_render(512);
    int32_t env_before = 0;
    for (i = 0; i < NVOICE; i++) if (t->v[i].gate) env_before = t->v[i].env;
    mo0 = mo_w;
    ambient_tap_key(14);
    balanced = mo_w == mo0 + 2 && ((midi_out_q[mo0 % MQ] >> 16) & 127u) == 60 &&
        ((midi_out_q[mo0 % MQ] >> 8) & 0xF0u) == 0x80 &&
        ((midi_out_q[(mo0 + 1) % MQ] >> 16) & 127u) == 67;
    bad += ambient_check("register moves one held voice and balances old MIDI pitch",
        ambient_layers[1].notes[0] == 67 && ambient_gated_notes(t) == 1 && !gate_note(t, 60) && balanced);
    int32_t env_after = 0;
    for (i = 0; i < NVOICE; i++) if (t->v[i].gate) env_after = t->v[i].env;
    bad += ambient_check("register change preserves a nonzero envelope without retrigger gap", env_before > 0 && env_after == env_before);
    press(B_PLAY); events_block(CTL);
    bad += ambient_check("PLAY starts transport without clearing either held layer", song.playing && ambient_layers[1].count && ambient_layers[0].count == 4);
    press(B_PLAY); events_block(CTL);
    midi_note_event(1, 67, 100); midi_note_event(1, 67, 0);
    bad += ambient_check("external note-off cannot release physical TEXTURE", gate_note(t, 67) && ambient_layers[1].count);
    midi_note_event(1, 67, 100);
    ambient_tap_key(7);
    bad += ambient_check("moving register retains an externally owned old pitch", gate_note(t, 60) && gate_note(t, 67));
    midi_note_event(1, 67, 0);
    bad += ambient_check("old external pitch releases independently", !gate_note(t, 67) && gate_note(t, 60));
    ambient_tap_key(7);
    bad += ambient_check("same mapped note toggles TEXTURE off", !ambient_layers[1].count && !ambient_gated_notes(t));
    ambient_tap_key(7);
    memcpy(params, trk[0].p, sizeof params); memcpy(global, song.g, sizeof global);
    turn(EN_K1, 128); turn(EN_K2, 128); turn(EN_K3, 128); turn(EN_K4, 128);
    isolated = !memcmp(params, trk[0].p, sizeof params) && !memcmp(global, song.g, sizeof global);
    bad += ambient_check("TEXTURE macros preserve all TIDE parameters and shared FX settings", isolated &&
        ambient_layers[1].shape == 100 && ambient_layers[1].motion == 100 && ambient_layers[1].density == 100 && ambient_layers[1].space == 100);
    memcpy(params, t->p, sizeof params);
    int16_t mix_tide_before = trk[0].p[P_LEVEL], mix_texture_before = t->p[P_LEVEL];
    btn_down(B_HOME); frame();
    turn(EN_PRESET, 1); /* MIX must not accidentally replace the sound. */
    turn(EN_K1, -1); turn(EN_K2, 1); turn(EN_K3, -128); turn(EN_K4, -128);
    isolated = 1;
    for (i = 0; i < P_COUNT; i++) if (i != P_LEVEL) isolated &= params[i] == t->p[i];
    frames(900);
    bad += ambient_check("HOME MIX owns knobs immediately and suppresses macros/menu", ambient_mix && !ui.menu &&
        trk[0].p[P_LEVEL] == mix_tide_before - 1 && t->p[P_LEVEL] == mix_texture_before + 1 && isolated);
    btn_up(B_HOME); frame();
    bad += ambient_check("MIX release restores macros without navigation", !ambient_mix && !ui.menu && ui.home);
    btn_down(B_FX); frame(); btn_down(B_HOME); frame();
    int16_t level_before = t->p[P_LEVEL];
    turn(EN_K2, 1); frames(800);
    bad += ambient_check("MIX owns knobs/display even with FX quick-layer held", ambient_mix && ui.home &&
        t->p[P_LEVEL] == level_before + 1 && ambient_layers[1].space == 100 && !ui.menu);
    btn_up(B_HOME); frame(); btn_up(B_FX); frame();
    bad += ambient_check("FX/MIX overlapping releases neither open menu nor edit macros", ui.home && !ui.menu && !ambient_mix && ambient_layers[1].space == 100);
    frames(LY_QUIET_MS + 32); /* Preserve Felucca's late-detent guard after FX release. */
    turn(EN_K4, -1);
    bad += ambient_check("macro edits resume after MIX release", ambient_layers[1].space == 99);
    turn(EN_ALGO, 1);
    bad += ambient_check("ambient HOME selection reaches PULSE", song.sel == 2);
    turn(EN_ALGO, -1); turn(EN_ALGO, -1);
    texture_clear();
    bad += ambient_check("clearing TIDE preserves TEXTURE", !ambient_layers[0].count && !ambient_gated_notes(&trk[0]) && ambient_layers[1].count && gate_note(t, 60));
    ambient_tap_key(7); turn(EN_ALGO, 1); texture_clear();
    bad += ambient_check("clearing TEXTURE preserves TIDE", !ambient_layers[1].count && !ambient_gated_notes(t) && ambient_layers[0].count == 1 && gate_note(&trk[0], 60));
    press(B_EDIT); turn(EN_ALGO, 1);
    bad += ambient_check("full editing UI retains access to spare tracks", !ui.home && song.sel == 2);
    hold(B_HOME);
    bad += ambient_check("full UI HOME hold still opens menu", ui.menu);
    press(B_HOME); turn(EN_ALGO, -1); ambient_tap_key(7);
    hold(B_HOME);
    bad += ambient_check("ambient HOME long hold ends MIX and stays HOME", !ui.menu && ui.home && !ambient_mix);
    /* Real render blocks exercise the shared allocator with long release tails. */
    ambient_audio_reset();
    ambient_tap_key(7); ambient_tap_key(9); ambient_tap_key(11); ambient_tap_key(12);
    song.sel = 1; ambient_tap_key(7); texture_render(512);
    for (j = 0; j < 40; j++) {
        song.sel = 0;
        ambient_latch_clear(&trk[0]);
        for (i = 0; i < 4; i++) ambient_latch_toggle(&trk[0], 48 + j % 12 + i * 3, 100);
        song.sel = 1; ambient_tap_key(j % 2 ? 7 : 14);
        texture_render(4);
        stable &= ambient_gated_notes(&trk[0]) == 4 && ambient_gated_notes(t) == 1 && voices_busy() <= NVOICE;
    }
    bad += ambient_check("repeated chords/registers preserve five held voices under release pressure", stable);
    {
        int32_t old = ambient_level[0];
        trk[0].p[P_LEVEL] = 0; texture_render(1);
        bad += ambient_check("MIX hard fade slews gain without killing held voices",
            ambient_level[0] > 0 && ambient_level[0] < old && ambient_gated_notes(&trk[0]) == 4);
        texture_render(256);
        bad += ambient_check("MIX fade reaches exact silence", ambient_level[0] == 0);
        trk[0].p[P_LEVEL] = 127; trk[1].p[P_LEVEL] = 127; texture_render(256);
        bad += ambient_check("MIX gain recovers without allocating more held voices",
            ambient_level[0] == LEVEL_Q12[127] && ambient_gated_notes(&trk[0]) == 4);
    }
    for (i = 80; i < 92; i++) midi_note_event(1, i, 100);
    texture_render(4);
    printf("texture MIDI pressure: 12 external pitches, TIDE gated %u/4, physical TEXTURE sounding %d, budget %u/%u\n",
        ambient_gated_notes(&trk[0]), gate_note(t, ambient_layers[1].notes[0]), voices_busy(), NVOICE);
    bad += ambient_check("external MIDI overload remains within shared voice budget", voices_busy() <= NVOICE);
    midi_control(1, 123, 0); panic_req = 15; events_block(CTL);
    bad += ambient_check("MIDI overload and clear do not leave gated voices", !ambient_gated_notes(&trk[0]) && !ambient_gated_notes(t));
    ambient_boot_apply();
    bad += ambient_check("reset clears both holds and restores independent macro defaults", !ambient_layers[1].count && !ambient_layers[0].count &&
        ambient_layers[1].shape == 35 && ambient_layers[1].space == 50 && ambient_layers[0].density == 50);
    song.sel = 1; ambient_tap_key(7); press(B_EDIT); turn(EN_PRESET, 1); events_block(CTL);
    bad += ambient_check("loading a sound forgets TEXTURE and balances its hold", !ambient_layers[1].count && !texture_active(t));
    ambient_audio_reset();
    song.sel = 1; ambient_tap_key(7);
    song.sel = 0; press(B_EDIT); turn(EN_PRESET, 1); events_block(CTL);
    bad += ambient_check("loading TIDE leaves physical TEXTURE owned and sounding", ambient_layers[1].count && gate_note(t, 60) && !ambient_home_active(&trk[0]));
    song.sel = 1; press(B_HOME); texture_clear();
    bad += ambient_check("TEXTURE clear works after the other layer changes sound", !ambient_layers[1].count && !ambient_gated_notes(t));
    bad += texture_audio_checks();
    printf(bad ? "texture: %d FAILED\n" : "texture: all passed\n", bad);
    return bad ? 1 : 0;
}
