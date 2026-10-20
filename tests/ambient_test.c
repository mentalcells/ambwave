/* SPDX-License-Identifier: GPL-3.0-only */
/* Focused host checks for AmbWave's AMBIENT_POC power-on scene. */
#define AMBIENT_POC 1
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static int ambient_check(const char *what, int ok)
{
    printf("ambient: %-66s %s\n", what, ok ? "ok" : "FAIL");
    return ok ? 0 : 1;
}

static int gate_note(const track_t *t, uint32_t note)
{
    uint32_t i;
    for (i = 0; i < NVOICE; i++)
        if (t->v[i].active && t->v[i].gate && t->v[i].note == note)
            return 1;
    return 0;
}

static uint32_t ambient_gated_notes(const track_t *t)
{
    uint32_t i, n = 0;
    for (i = 0; i < NVOICE; i++)
        n += t->v[i].active && t->v[i].gate;
    return n;
}

static void ambient_tap_key(uint32_t key)
{
    key_down(key);
    key_up(key);
}

typedef struct {
    double held, early, late;
    int32_t peak;
} ambient_audio_measure_t;

static void ambient_audio_reset(void)
{
    memset(dly_buf, 0, sizeof dly_buf);
    memset(cho_buf, 0, sizeof cho_buf);
    memset(rev_comb, 0, sizeof rev_comb);
    memset(&rev_u, 0, sizeof rev_u);
    memset(&fx, 0, sizeof fx);
    lim_env = LIM_T;
    lc_l1 = lc_l2 = lc_r1 = lc_r2 = dc_l = dc_r = dce_l = dce_r = 0;
    fm1_in.notes = fm1_in.buttons = 0;
    kb_prev = 0;
    ui_power_on();
    song.master_q12 = 4096;
}

static ambient_audio_measure_t ambient_audio_render(uint32_t amount, int density)
{
    ambient_audio_measure_t m = {0, 0, 0, 0};
    uint32_t f, i, nh = 0, ne = 0, nl = 0, released = 0;
    int32_t out[2 * CTL];
    ambient_audio_reset();
    /* Measure macro response at the same reference fader as before the
     * reviewed boot-mix change. Keep the absolute tail criterion intact;
     * the factory mixer levels are checked independently below. */
    if (!density) {
        trk[0].p[P_LEVEL] = 100;
        ambient_level[0] = LEVEL_Q12[100];
    }
    if (density) {
        ambient_layers[0].density = (uint8_t)amount;
        ambient_density_apply(&trk[0]);
    } else {
        ambient_layers[0].space = (uint8_t)amount;
        ambient_space_apply(&trk[0]);
    }
    trk_note_on(&trk[0], 48, 92);
    trk_note_on(&trk[0], 55, 92);
    trk_note_on(&trk[0], 64, 92);
    if (density) trk_note_on(&trk[0], 67, 92);
    for (f = 0; f < 8u * FS; f += CTL) {
        if (!released && f + CTL >= 2u * FS) {
            trk_all_off(&trk[0]);
            /* Isolate the effect tail from TIDE's deliberately long release. */
            if (!density) for (i = 0; i < NVOICE; i++) trk[0].v[i].active = 0;
            released = 1;
        }
        mix_block(out, CTL);
        for (i = 0; i < 2u * CTL; i++) {
            int32_t a = out[i] < 0 ? -out[i] : out[i];
            double e = (double)out[i] * out[i];
            if (a > m.peak) m.peak = a;
            if (f >= FS && f < 2u * FS) { m.held += e; nh++; }
            if (f >= 3u * FS && f < 4u * FS) { m.early += e; ne++; }
            if (f >= 6u * FS && f < 8u * FS) { m.late += e; nl++; }
        }
    }
    m.held = sqrt(m.held / nh);
    m.early = sqrt(m.early / ne);
    m.late = sqrt(m.late / nl);
    return m;
}


#ifndef AMBIENT_REFERENCE_MAIN
#define AMBIENT_REFERENCE_MAIN main
#endif
int AMBIENT_REFERENCE_MAIN(void)
{
    static const int16_t expected[] = {
        0, 0, 0, 88, 96, 120, 112, 82, 0, 42, 82
    };
    track_t *t;
    uint32_t i, mo0;
    int bad = 0, empty = 1, muted = 1, params;

    setvbuf(stdout, NULL, _IONBF, 0);
    ui_power_on();
    t = &trk[0];

    for (i = 0; i < NTRK; i++)
        empty &= seq_is_empty(&trk[i]) && pat_last[i] == 0u && pat_sig[i] == steps_sig(&trk[i]);
    for (i = 3; i < NTRK; i++)
        muted &= !trk[i].p[P_MUTE] && cloud_active(&trk[i]);

    bad += ambient_check("track 1 selected, FM6 / TIDE BODY, no boot undo",
                         song.sel == 0u && t->eng_req == ENGI_FM6 &&
                         t->engine == ENGI_FM6 &&
                         t->preset == 4 && !t->p[P_MUTE] && undo.trk == 0 && undo_depth == 0);
    bad += ambient_check("four curated layers available, all sequencers empty", muted && empty);
    bad += ambient_check("boot mixer retains the user-reviewed layered balance",
                         trk[0].p[P_LEVEL] == 82 && trk[1].p[P_LEVEL] == 80 &&
                         trk[2].p[P_LEVEL] == 94 && trk[3].p[P_LEVEL] == 80);
    bad += ambient_check("transport and recording remain stopped",
                         !song.playing && !song.rec && !transport_req);

    params = t->p[P_E1] == expected[0] && t->p[P_E4] == expected[1] && t->p[P_E5] == expected[2] &&
             t->p[P_ATK] == expected[3] && t->p[P_DEC] == expected[4] && t->p[P_SUS] == expected[5] &&
             t->p[P_REL] == expected[6] && t->p[P_LEVEL] == expected[7] && t->p[P_CHOR] == expected[8] &&
             t->p[P_DLY] == expected[9] && t->p[P_REV] == expected[10];
    bad += ambient_check("curated drone parameters applied", params && song.g[G_DTIME] == 1 &&
                         song.g[G_DFDBK] == 54 && song.g[G_DCOLOR] == 62 && song.g[G_DMIX] == 82 &&
                         song.g[G_RSIZE] == 108 && song.g[G_RDAMP] == 76 &&
                         song.g[G_CRATE] == 32 && song.g[G_CDEPTH] == 52);

    bad += ambient_check("HOME identifies the named TIDE scene and empty four-note latch",
                         ui.home && ambient_home_active(t) && str_eq(ambient_patch_name(), "BODY PAD") &&
                         ambient_latch_count() == 0u);
    {
        int16_t before[P_COUNT];
        memcpy(before, t->p, sizeof before);
        bad += ambient_check("DENSITY default retains heard FM patch", ambient_density_amount(t) == 50 && !ambient_tide_body(t) && !t->p[P_E6] && !t->p[P_CHOR]);
        turn(EN_K3, -127);
        bad += ambient_check("DENSITY lowers harmonic body", ambient_density_amount(t) == 0 && ambient_tide_body(t) == -(2 << 24));
        turn(EN_K3, 127);
        bad += ambient_check("DENSITY raises harmonic body without detuning", ambient_density_amount(t) == 100 && ambient_tide_body(t) == (2 << 24));
        bad += ambient_check("DENSITY preserves all track parameters", !memcmp(before, t->p, sizeof before));
        ambient_boot_apply();

    }
    press(B_ENV);
    bad += ambient_check("a page-family key escapes to the complete Felucca UI",
                         !ui.home && cur_page()->fam == FAM_ENV);
    press(B_HOME);
    bad += ambient_check("HOME returns to the focused ambient performance screen",
                         ui.home && ambient_home_active(t));

    {
        bad += ambient_check("SHAPE starts at heard FM patch", ambient_shape_amount(t) == 50 && t->p[P_E2] == 0);
        turn(EN_K1, -200);
        bad += ambient_check("SHAPE reaches soft FM bound", ambient_shape_amount(t) == 0 && t->p[P_E2] == -16);
        turn(EN_K1, 1);
        bad += ambient_check("SHAPE retains fine macro steps", ambient_shape_amount(t) == 1);
        turn(EN_K1, 200);
        bad += ambient_check("SHAPE reaches brighter FM bound", ambient_shape_amount(t) == 100 && t->p[P_E2] == 12);
        ambient_boot_apply();

    }

    {
        track_t before = *t;
        uint32_t j;
        int untouched = 1;
        bad += ambient_check("MOTION boots as smooth FM index drift",
                             ambient_motion_amount(t) == AMBIENT_MOTION_DEFAULT &&
                             t->p[P_LRATE] == 9 && t->p[P_LWAVE] == 5 && t->p[P_LD_FLT] == 10 &&
                             t->p[P_LD_PIT] == 0 && t->p[P_LD_SHP] == 0 && t->p[P_LD_AMP] == 0);
        turn(EN_K2, -200);
        bad += ambient_check("KNOB 2 reaches a completely still MOTION bound",
                             ambient_motion_amount(t) == 0 && t->p[P_LRATE] == 6 && t->p[P_LD_FLT] == 0);
        turn(EN_K2, 50);
        for (j = 0; j < P_COUNT; j++)
            if (j != P_LRATE && j != P_LWAVE && j != P_LD_PIT && j != P_LD_FLT &&
                j != P_LD_SHP && j != P_LD_AMP)
                untouched &= t->p[j] == before.p[j];
        bad += ambient_check("MOTION rises monotonically and edits only the intended LFO values",
                             ambient_motion_amount(t) == 50 && t->p[P_LRATE] == 16 &&
                             t->p[P_LD_FLT] == 14 && untouched);
        turn(EN_K2, 200);
        bad += ambient_check("MOTION reaches a bounded animated top without pitch wobble",
                             ambient_motion_amount(t) == 100 && t->p[P_LRATE] == 30 &&
                             t->p[P_LD_FLT] == 24 && t->p[P_LD_PIT] == 0);
        ambient_boot_apply();
        bad += ambient_check("ambient scene reset restores the MOTION default",
                             ambient_motion_amount(t) == AMBIENT_MOTION_DEFAULT &&
                             t->p[P_LRATE] == 9 && t->p[P_LWAVE] == 5 && t->p[P_LD_FLT] == 10);
        {
            track_t drift = {0};
            uint32_t slow, fast;
            drift.p[P_LWAVE] = 5;
            drift.p[P_LRATE] = 12;
            drift.lfo_rnd_prev = 0x40000000u;           /* -16384 */
            drift.lfo_rnd = 0xC0000000u;                /* +16384 */
            bad += ambient_check("DRIFT eases continuously between random destinations",
                                 lfo_wave(&drift, 0) == -16384 &&
                                 abs(lfo_wave(&drift, 0x80000000u)) <= 1 &&
                                 lfo_wave(&drift, 0xFFFFFFFFu) >= 16382);
            drift.lfo_rnd = 0;
            slow = track_lfo_inc(&drift);
            drift.lfo_rnd = 191u;
            fast = track_lfo_inc(&drift);
            bad += ambient_check("DRIFT varies glide duration around the nominal rate",
                                 slow < LFO_INC[12] && fast > LFO_INC[12] && fast > slow * 2u);
        }
    }

    {
        track_t before = *t;
        int16_t globals[G_COUNT];
        int untouched = 1;
        memcpy(globals, song.g, sizeof globals);
        bad += ambient_check("SPACE boots at the original TIDE tuning with room above it",
                             ambient_space_amount(t) == AMBIENT_SPACE_DEFAULT && t->p[P_DLY] == 42 &&
                             t->p[P_REV] == 82 && song.g[G_DMIX] == 82 && song.g[G_RSIZE] == 108);
        turn(EN_K4, -200);
        bad += ambient_check("KNOB 4 reaches the close, nearly dry SPACE bound",
                             ambient_space_amount(t) == 0 && t->p[P_DLY] == 0 && t->p[P_REV] == 0 &&
                             song.g[G_DTIME] == 1 && song.g[G_DFDBK] == 54 &&
                             song.g[G_DMIX] == 82 && song.g[G_RSIZE] == 108 &&
                             song.g[G_RDAMP] == 76 && ambient_space_dry_gain(t) == 32767);
        turn(EN_K4, 40);
        for (i = 0; i < P_COUNT; i++)
            if (i != P_DLY && i != P_REV)
                untouched &= t->p[i] == before.p[i];
        for (i = 0; i < G_COUNT; i++) untouched &= song.g[i] == globals[i];
        bad += ambient_check("SPACE rises monotonically and edits only its intended effect values",
                             ambient_space_amount(t) == 40 && t->p[P_DLY] == 14 && t->p[P_REV] == 65 &&
                             song.g[G_DTIME] == 1 && song.g[G_DFDBK] == 54 &&
                             song.g[G_DMIX] == 82 && song.g[G_RSIZE] == 108 &&
                             song.g[G_RDAMP] == 76 && ambient_space_dry_gain(t) == 32767 && untouched);
        turn(EN_K4, 200);
        bad += ambient_check("KNOB 4 reaches the safe wide SPACE bound",
                             ambient_space_amount(t) == 100 && t->p[P_DLY] == 110 && t->p[P_REV] == 127 &&
                             song.g[G_DTIME] == 1 && song.g[G_DFDBK] == 54 &&
                             song.g[G_DMIX] == 82 && song.g[G_RSIZE] == 108 &&
                             song.g[G_RDAMP] == 76 && ambient_space_dry_gain(t) == 18400);
        ambient_boot_apply();
        bad += ambient_check("ambient scene reset restores the SPACE default",
                             ambient_space_amount(t) == AMBIENT_SPACE_DEFAULT && t->p[P_DLY] == 42 &&
                             t->p[P_REV] == 82 && song.g[G_DMIX] == 82 && song.g[G_RSIZE] == 108);
    }

    {
        ambient_audio_measure_t close = ambient_audio_render(0, 0);
        ambient_audio_measure_t tide = ambient_audio_render(AMBIENT_SPACE_DEFAULT, 0);
        ambient_audio_measure_t wash = ambient_audio_render(100, 0);
        printf("ambient: SPACE render RMS held %.0f/%.0f/%.0f, early %.0f/%.0f/%.0f, late %.0f/%.0f/%.0f\n",
               close.held, tide.held, wash.held, close.early, tide.early, wash.early,
               close.late, tide.late, wash.late);
        bad += ambient_check("SPACE render has a progressive, plainly stronger effect tail",
                             close.early < 1 && tide.early > 50 && wash.early > tide.early * 1.30 &&
                             close.late < 1 && tide.late < 1 && wash.late < 1);
        bad += ambient_check("SPACE render stays bounded at every tested endpoint",
                             close.peak <= 32767 && tide.peak <= 32767 && wash.peak <= 32767);
    }
    ui_power_on();
    t = &trk[0];

    usb.config = 1; mo_w = mo_r = 0;
    ambient_tap_key(7);                                           /* C4 */
    bad += ambient_check("physical key adds a latch while transport stays stopped",
                         ambient_layers[0].count == 1u && gate_note(t, 60) && !song.playing && !transport_req);
    bad += ambient_check("physical key-up leaves the latched note held", gate_note(t, 60));
    bad += ambient_check("a released latched note keeps its physical key LED lit",
                         ambient_latch_leds(t) == (1u << 7));
    ambient_tap_key(7);
    bad += ambient_check("pressing the latched key again removes it",
                         !ambient_layers[0].count && !gate_note(t, 60) && !ambient_latch_leds(t));
    ambient_tap_key(7);
    bad += ambient_check("a removed note can be added again",
                         ambient_layers[0].count == 1u && gate_note(t, 60));

    ambient_tap_key(9); ambient_tap_key(11); ambient_tap_key(12); /* D4 E4 F4: four total */
    ambient_tap_key(14);                                          /* G4: rejected */
    bad += ambient_check("the ambient latch is capped at four notes",
                         ambient_layers[0].count == AMBIENT_LATCH_MAX && ambient_gated_notes(t) == AMBIENT_LATCH_MAX &&
                         !gate_note(t, 67) && ambient_latch_leds(t) ==
                         ((1u << 7) | (1u << 9) | (1u << 11) | (1u << 12)));

    mo0 = mo_w;
    btn_down(B_FX); frame(); press(B_OCTDN); btn_up(B_FX); frame();
    events_block(CTL);
    bad += ambient_check("FX + OCT- clears every latched note",
                         !ambient_layers[0].count && !ambient_gated_notes(t) && msg_is("ALL NOTES OFF"));
    {
        static const uint8_t notes[] = {60, 62, 64, 65};
        int balanced = mo_w == mo0 + NELEM(notes);
        for (i = 0; i < NELEM(notes) && balanced; i++)
            balanced &= ((midi_out_q[(mo0 + i) % MQ] >> 8) & 0xF0u) == 0x80u &&
                        ((midi_out_q[(mo0 + i) % MQ] >> 16) & 127u) == notes[i];
        bad += ambient_check("FX + OCT- sends MIDI note-offs for every latched pitch", balanced);
    }

    ambient_tap_key(7);
    ambient_boot_apply();
    bad += ambient_check("ambient scene reinitialization clears the latch",
                         ambient_latch_enabled && !ambient_layers[0].count && !ambient_gated_notes(t));
    ambient_tap_key(7);
    ambient_latch_enable(0);
    bad += ambient_check("disabling the latch releases its held notes",
                         !ambient_latch_enabled && !ambient_layers[0].count && !ambient_gated_notes(t));
    ambient_latch_enable(1);
    ambient_tap_key(7);
    midi_control(0, 123, 0);
    bad += ambient_check("MIDI All Notes Off forgets and releases the latch",
                         !ambient_layers[0].count && !ambient_gated_notes(t));
    ambient_tap_key(7);
    panic_req |= 1u;
    events_block(CTL);
    bad += ambient_check("the existing panic path forgets and releases the latch",
                         !ambient_layers[0].count && !ambient_gated_notes(t));

    midi_note_event(0, 64, 100);                                  /* USB/TRS share this endpoint */
    bad += ambient_check("MIDI note-on remains momentary and independently owned", gate_note(t, 64));
    midi_note_event(0, 64, 0);
    bad += ambient_check("MIDI note-off still releases a non-latched note", !gate_note(t, 64));
    ambient_tap_key(7);
    midi_note_event(0, 60, 100);
    midi_note_event(0, 60, 0);
    bad += ambient_check("MIDI note-off cannot release a physical latch",
                         ambient_layers[0].count == 1u && gate_note(t, 60));
    ambient_tap_key(7);

    song.sel = 3;
    set_engine_of(&trk[3], 0); apply_preset_to(&trk[3], 1); events_block(CTL);
    trk[3].p[P_MUTE] = 0;                                      /* expose its unchanged normal input path */
    key_down(7);
    bad += ambient_check("physical keys on non-ambient tracks remain momentary",
                         ambient_layers[0].count == 0u && gate_note(&trk[3], 60));
    key_up(7);
    bad += ambient_check("non-ambient track key-up still releases its note", !gate_note(&trk[3], 60));
    song.sel = 0;

    bad += ambient_check("AmbWave splash identity preserved",
                         str_eq(FELUCCA_PRODUCT, "AMBWAVE") && str_eq(FELUCCA_TAGLINE, "AMBIENT MVP"));
    {
        ambient_audio_measure_t thin = ambient_audio_render(0, 1);
        ambient_audio_measure_t tide = ambient_audio_render(50, 1);
        ambient_audio_measure_t broad = ambient_audio_render(100, 1);
        printf("ambient: DENSITY RMS thin/tide/broad %.1f / %.1f / %.1f, peaks %d / %d / %d\n",
               thin.held, tide.held, broad.held, thin.peak, tide.peak, broad.peak);
        bad += ambient_check("DENSITY renders audible bounded chords across its range",
                             thin.held > 100 && tide.held > 100 && broad.held > 100 &&
                             thin.peak < 32767 && tide.peak < 32767 && broad.peak < 32767);
        bad += ambient_check("DENSITY avoids a large loudness jump at either endpoint",
                             thin.held < tide.held * 2 && broad.held < tide.held * 2);
    }
    printf(bad ? "ambient: %d FAILED\n" : "ambient: all passed\n", bad);
    return bad ? 1 : 0;
}
