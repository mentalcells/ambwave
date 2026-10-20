/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN

static void audition_blocks(uint32_t n)
{ int32_t out[CTL * 2]; while (n--) mix_block(out, CTL); }

static int factory_matches(track_t *t, const int16_t *base, const uint8_t *fm)
{
    for (uint32_t p = 0; p < P_COUNT; p++)
        if (!param_kept(p) && t->p[p] != base[p]) return 0;
    return t->eng_req != ENGI_FM6 || !memcmp(fm, fm6_patch[t - trk], FP_SIZE);
}

static int check_factory_integrity(void)
{
    int bad = 0;
    for (uint32_t entry = 0; entry < NELEM(ambient_patches); entry++) {
        const ambient_patch_def_t *def = &ambient_patches[entry];
        if (def->factory == 255 || def->role == AS_DRONE || (def->role == AS_TIDE && def->id >= 8)) continue;
        for (uint32_t k = 0; k < NTRK; k++) for (uint32_t mode = 0; mode < 3; mode++) {
            ambient_audio_reset(); song.sel = k;
            track_t *t = &trk[k];
            t->eng_req = def->engine; apply_preset_to(t, def->factory);
            int16_t base[P_COUNT], globals[G_COUNT]; uint8_t fm[FP_SIZE + 1];
            memcpy(base, t->p, sizeof base); memcpy(fm, fm6_patch[k], sizeof fm);
            memcpy(globals, song.g, sizeof globals);
            /* Seed a nonempty phrase and track settings before the curated load. */
            t->step[0].n = 1; t->step[0].time = ST_NOTE; t->step[0].note[0] = 67;
            t->p[P_LEVEL] = 59; t->p[P_PAN] = -9; ambient_muted = 1u << k;
            ambient_layers[k].mode = mode; ambient_layers[k].speed = 3; ambient_layers[k].speed_set = 1;
            ambient_patch_apply(t, def->role, def->id);
            bad += ambient_check("factory audition preserves source, ADSR/operator envelopes and sends",
                ambient_home_active(t) && factory_matches(t, base, fm));
            bad += ambient_check("candidate load retains mode speed pattern mix mute and shared effects",
                ambient_layers[k].mode == mode && ambient_layers[k].speed == 3 &&
                t->p[P_ARATE] == pulse_rates[3] && t->step[0].note[0] == 67 &&
                t->p[P_LEVEL] == 59 && t->p[P_PAN] == -9 && ambient_muted == (1u << k) &&
                !memcmp(globals, song.g, sizeof globals));
            ambient_shape_turn(t, 100); ambient_motion_turn(t, 100);
            if (def->role == AS_TIDE) ambient_density_turn(t, 100);
            bad += ambient_check("unmapped audition macros cannot reinterpret another engine's parameters",
                factory_matches(t, base, fm) && ambient_layers[k].shape == 50 && !ambient_layers[k].motion);
            if (def->role == AS_PULSE) {
                int16_t before[P_COUNT]; memcpy(before, t->p, sizeof before);
                ambient_density_turn(t, 100);
                int only_timing = t->p[P_APROB] == 127;
                for (uint32_t p = 0; p < P_COUNT; p++) if (p != P_APROB) only_timing &= before[p] == t->p[p];
                bad += ambient_check("factory Pulse density changes probability only", only_timing && factory_matches(t, base, fm));
            }
            ambient_space_turn(t, -100);
            bad += ambient_check("candidate SPACE zero bypasses all three spatial sends",
                !t->p[P_CHOR] && !t->p[P_DLY] && !t->p[P_REV] && ambient_layers[k].dry == 32767);
            ambient_space_turn(t, 50);
            bad += ambient_check("candidate SPACE midpoint restores original factory sends exactly", factory_matches(t, base, fm));
            for (uint32_t next = 0; next < 3; next++) {
                ambient_mode_set(t, next);
                bad += ambient_check("mode changes never rewrite candidate source envelopes or effects", factory_matches(t, base, fm));
            }
        }
    }
    return bad;
}

static uint32_t factory_dry_hash(const ambient_patch_def_t *def, int curated)
{
    ambient_audio_reset(); track_t *t = &trk[0];
    if (curated) ambient_patch_apply(t, def->role, def->id);
    else { t->eng_req = def->engine; apply_preset_to(t, def->factory); }
    t->p[P_LEVEL] = 100; t->p[P_PAN] = 0;
    ambient_level[0] = LEVEL_Q12[100];
    formant_nz = 0x2545F491;
    t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    audition_blocks(100);
    vage = 0; /* ANALOG noise is seeded from voice age; compare the same realization. */
    trk_note_on(t, 60, 100);
    uint32_t hash = 2166136261u;
    for (uint32_t f = 0; f < 2 * FS; f += CTL) {
        int32_t out[CTL * 2]; mix_block(out, CTL); hash = fnv(hash, out, sizeof out);
    }
    return hash;
}

static int check_factory_audio_and_projects(void)
{
    int bad = 0;
    for (uint32_t entry = 0; entry < NELEM(ambient_patches); entry++) {
        const ambient_patch_def_t *def = &ambient_patches[entry];
        if (def->factory == 255 || def->role == AS_DRONE || (def->role == AS_TIDE && def->id >= 8)) continue;
        uint32_t plain = factory_dry_hash(def, 0), curated = factory_dry_hash(def, 1);
        printf("factory dry %s: plain %08x curated %08x\n",def->name,plain,curated);
        bad += ambient_check("candidate dry audio equals a plain factory load bit for bit", plain == curated);
        ambient_audio_reset(); ambient_patch_apply(&trk[0], def->role, def->id);
        audition_blocks(100);
        int16_t base[P_COUNT]; uint8_t fm[FP_SIZE + 1];
        memcpy(base, trk[0].p, sizeof base); memcpy(fm, fm6_patch[0], sizeof fm);
        static project_t project; static project_store_t wire;
        project_capture(&project);
        int packed = proj_pack(&wire, &project);
        ambient_boot_apply();
        int unpacked = proj_unpack(&project, wire.raw, sizeof wire);
        int restored = project_restore_runtime(&project);
        audition_blocks(100);
        bad += ambient_check("existing project format round-trips candidate source and leaves curated mode",
            packed && unpacked && !restored && factory_matches(&trk[0], base, fm) && !ambient_home_active(&trk[0]));
    }
    return bad;
}

static int check_selection_and_playback(void)
{
    int bad = 0;
    for (uint32_t k = 0; k < NTRK; k++) for (uint32_t r = 0; r < 2; r++) {
        uint32_t role = r ? AS_PULSE : AS_TIDE, count = r ? 6 : 10;
        ambient_audio_reset(); song.sel = k; ambient_patch_apply(&trk[k], role, 0);
        for (uint32_t p = 1; p <= count; p++) {
            turn(EN_PRESET, 1); audition_blocks(100);
            bad += ambient_check("physical preset knob reaches every sound without leaving its role",
                ui.home && ambient_home_active(&trk[k]) && ambient_layers[k].role == role && ambient_layers[k].patch == p % count);
        }
    }
    for (uint32_t entry = 0; entry < NELEM(ambient_patches); entry++) {
        const ambient_patch_def_t *def = &ambient_patches[entry];
        if (def->factory == 255 || def->role == AS_DRONE || (def->role == AS_TIDE && def->id >= 8)) continue;
        for (uint32_t mode = 0; mode < 3; mode++) {
            ambient_audio_reset(); track_t *t = &trk[0];
            ambient_patch_apply(t, def->role, def->id); ambient_mode_set(t, mode); audition_blocks(100);
            t->step[0].n = 1; t->step[0].note[0] = 60; t->step[0].time = ST_NOTE;
            if (mode != AM_SEQUENCE) ambient_latch_toggle(t, 60, 100);
            seq_start(); int32_t peak = 0; double energy = 0;
            for (uint32_t f = 0; f < 3 * FS; f += CTL) {
                int32_t out[CTL * 2]; mix_block(out, CTL);
                for (uint32_t i = 0; i < CTL * 2; i++) {
                    int32_t a = abs(out[i]); if (a > peak) peak = a;
                    energy += (double)out[i] * out[i];
                }
            }
            bad += ambient_check("every factory candidate sounds in HOLD SCATTER and SEQUENCE", peak > 30 && peak < 32767 && energy > 1e6);
            seq_stop(); ambient_latch_clear(t); audition_blocks(45 * FS / CTL);
            uint32_t voices = 0; for (uint32_t v = 0; v < NVOICE; v++) voices += t->v[v].active;
            bad += ambient_check("factory voices release after STOP and clearing the held input", !voices && !t->seq_n && !t->nheld && !ambient_layers[0].count);
        }
    }
    /* Undo/redo includes the imported identity, factory parameters and source patch. */
    ambient_audio_reset(); song.sel = 0;
    ambient_patch_apply(&trk[0], AS_TIDE, 7); audition_blocks(100);
    int16_t choir[P_COUNT]; memcpy(choir, trk[0].p, sizeof choir);
    undo_swap(); audition_blocks(100);
    bad += ambient_check("factory undo returns to original Body Pad", ambient_home_active(&trk[0]) && !ambient_layers[0].patch);
    undo_swap(); audition_blocks(100);
    bad += ambient_check("factory redo restores Choir without old Tide overrides", ambient_home_active(&trk[0]) && ambient_layers[0].patch == 7 && !memcmp(choir, trk[0].p, sizeof choir));
    return bad;
}

static int check_four_track_cost(void)
{
    int bad = 0; double worst = 0;
    for (uint32_t entry = 0; entry < NELEM(ambient_patches); entry++) {
        const ambient_patch_def_t *def = &ambient_patches[entry];
        if (def->factory == 255 || def->role == AS_DRONE || (def->role == AS_TIDE && def->id >= 8)) continue;
        ambient_audio_reset();
        for (uint32_t k = 0; k < NTRK; k++) {
            ambient_patch_apply(&trk[k], def->role, def->id);
            ambient_mode_set(&trk[k], AM_HOLD); ambient_layers[k].space = 100; ambient_space_apply(&trk[k]);
            trk[k].p[P_LEVEL] = 100;
        }
        audition_blocks(100);
        for (uint32_t k = 0; k < NTRK; k++) { ambient_latch_toggle(&trk[k], 48 + 4*k, 100); ambient_latch_toggle(&trk[k], 55 + 4*k, 100); }
        uint64_t start = now_ns(); int32_t peak = 0; double energy = 0, dc = 0;
        for (uint32_t f = 0; f < 4 * FS; f += CTL) {
            int32_t out[CTL * 2]; mix_block(out, CTL);
            for (uint32_t i = 0; i < CTL * 2; i++) {
                int32_t a = abs(out[i]); if (a > peak) peak = a;
                energy += (double)out[i] * out[i]; dc += out[i];
            }
        }
        double cost = (now_ns() - start) / 4e9; if (cost > worst) worst = cost;
        uint32_t active = 0;
        for (uint32_t k = 0; k < NTRK; k++) for (uint32_t v = 0; v < NVOICE; v++) active += trk[k].v[v].active;
        printf("factory four tracks: %s peak %d DC %.1f voices %u cost %.4f real time\n",def->name,peak,dc/(8*FS),active,cost);
        bad += ambient_check("four duplicate candidates remain audible bounded and inside shared voice budget",
            energy > 1e6 && peak < 32767 && fabs(dc/(8*FS)) < 500 && active <= NVOICE && cost < 0.5);
    }
    /* New sounds in the original four-layer performance, not only duplicates. */
    for (uint32_t entry = 0; entry < NELEM(ambient_patches); entry++) {
        const ambient_patch_def_t *def = &ambient_patches[entry];
        if (def->factory == 255 || def->role == AS_DRONE || (def->role == AS_TIDE && def->id >= 8)) continue;
        ambient_audio_reset(); uint32_t selected = def->role == AS_TIDE ? 0 : 2;
        ambient_patch_apply(&trk[selected], def->role, def->id); audition_blocks(100);
        ambient_latch_toggle(&trk[0], 48, 100); ambient_latch_toggle(&trk[0], 55, 100);
        ambient_latch_toggle(&trk[0], 60, 100); ambient_latch_toggle(&trk[0], 64, 100);
        for (uint32_t k = 1; k < NTRK; k++) ambient_latch_toggle(&trk[k], 60 + k * 5, 100);
        seq_start(); int healthy = 1; double energy = 0;
        for (uint32_t f = 0; f < 4 * FS; f += CTL) {
            int32_t out[CTL * 2]; mix_block(out, CTL);
            if (f > FS) healthy &= ambient_gated_notes(&trk[0]) == 4 && ambient_gated_notes(&trk[1]) == 1 && ambient_gated_notes(&trk[3]) == 1;
            for (uint32_t i = 0; i < CTL * 2; i++) { healthy &= abs(out[i]) < 32767; energy += (double)out[i] * out[i]; }
        }
        bad += ambient_check("candidate in original four-layer mix preserves sustained gates and headroom", healthy && energy > 1e6);
    }
    printf("factory four-track worst host render cost %.4f real time\n",worst);
    return bad;
}

int main(void)
{
    int bad = check_factory_integrity() + check_factory_audio_and_projects() + check_selection_and_playback() + check_four_track_cost();
    return bad ? 1 : 0;
}
