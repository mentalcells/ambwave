/* SPDX-License-Identifier: GPL-3.0-only */
/* AmbWave: four independent layers over Felucca's existing engines. */
#ifndef AMBIENT_POC
#define AMBIENT_POC 0
#endif
#if AMBIENT_POC
enum {
    AMBIENT_ANALOG_ENGINE = 0u, AMBIENT_ANALOG_PRESET = 1u,
    AMBIENT_LATCH_MAX = 4u, AMBIENT_SHAPE_MIN = 32u,
    AMBIENT_SHAPE_DEFAULT = 52u, AMBIENT_SHAPE_MAX = 108u,
    AMBIENT_MOTION_DEFAULT = 25u, AMBIENT_DENSITY_DEFAULT = 50u,
    AMBIENT_SPACE_DEFAULT = 50u
};
static uint8_t ambient_latch_enabled;
static int32_t ambient_two_stage(int32_t a, int32_t middle, int32_t top);
static int32_t ambient_level[NTRK];
static const char *const pulse_timbre_names[] = {"WARM PLUCK", "SOFT MALLET", "BELL PLUCK"};
static const char *const tide_timbre_names[] = {"TIDE", "TIDE WARM", "TIDE SILK"};
static const uint8_t pulse_shape_min[] = {2, 4, 8};
static const uint8_t pulse_shape_span[] = {90, 96, 96};
static const uint8_t pulse_rates[] = {6, 0, 1, 2};
static const char *const pulse_rate_names[] = {"1/2", "1/4", "1/8", "1/16"};
static ambient_layer_t *ambient_layer(const track_t *t) { return &ambient_layers[t - trk]; }
static int ambient_fm_pad(const track_t *t)
{ return ambient_layer(t)->sound == AS_TIDE && ambient_layer(t)->timbre != 1; }
static int ambient_grounded_pad(const track_t *t)
{ return ambient_layer(t)->sound == AS_TIDE && ambient_layer(t)->timbre == 0; }
static int32_t ambient_tide_body(const track_t *t)
{
    if (!ambient_latch_enabled || !ambient_grounded_pad(t) || t->eng_req != ENGI_FM6) return 0;
    return ((int32_t)ambient_layer(t)->density - 50) * (1 << 24) / 25;
}
static int ambient_warm_pad(const track_t *t)
{ return ambient_layer(t)->sound == AS_TIDE && ambient_layer(t)->timbre != 0; }
static int ambient_home_active(const track_t *t)
{
    static const uint8_t engines[] = {255, 0, 11, 2, 8};
    uint32_t sound = ambient_layer(t)->sound;
    return ambient_latch_enabled && sound && sound <= AS_CLOUD &&
        t->eng_req == (ambient_fm_pad(t) ? ENGI_FM6 : engines[sound]);
}
static int pulse_active(const track_t *t) { return ambient_home_active(t) && ambient_layer(t)->sound == AS_PULSE; }
static int ambient_scatter_active(const track_t *t) { return ambient_home_active(t) && ambient_layer(t)->mode == AM_SCATTER; }
static const char *ambient_mode_name(const track_t *t)
{
    static const char *const names[] = {"HOLD", "SCAT", "SEQ"};
    return names[ambient_layer(t)->mode];
}
static int cloud_active(const track_t *t) { return ambient_home_active(t) && ambient_layer(t)->sound == AS_CLOUD; }
/* TEXTURE/CLOUD share macro families; their HOLD voice limits differ. */
static int texture_active(const track_t *t)
{
    return ambient_home_active(t) && (ambient_layer(t)->sound == AS_TEXTURE || cloud_active(t));
}
/* Transport gestures occur once, not in the per-sample loops. Keep the
 * bounded four-slot search outside the compiler-inlined audio ISR body. */
static __attribute__((noinline)) void ambient_pulse_transport(int start)
{
    for (uint32_t i = 0; i < NTRK; i++) if (ambient_scatter_active(&trk[i])) {
        track_t *t = &trk[i];
        if (start && t->nheld) { t->arp_pos = 0xFFFFFFF; t->arp_idx = 0xFFFFFFFFu; }
        else if (!start && arp_sounding(t)) arp_off(t);
    }
}
static void ambient_sound_changed(const track_t *t)
{
    ambient_muted &= (uint8_t)~(1u << (t - trk));
    ambient_layer(t)->sound = AS_NONE;
}
static int ambient_shape_active(const track_t *t) { return ambient_home_active(t); }
static int ambient_latch_active(const track_t *t) { return ambient_home_active(t); }
static uint32_t ambient_latch_count(void) { return ambient_layers[0].count; }
static const char *ambient_layer_name(uint32_t k)
{
    static const char *const names[] = {"TRACK", "TIDE", "TEXTURE", "PULSE", "CLOUD"};
    static const char *const tracks[] = {"TRACK1", "TRACK2", "TRACK3", "TRACK4"};
    if (k < NTRK && ambient_layers[k].sound == AS_TIDE)
        return tide_timbre_names[ambient_layers[k].timbre];
    return k < NTRK ? (ambient_layers[k].sound ? names[ambient_layers[k].sound] : tracks[k]) : "TRACK";
}
static const char *ambient_layer_short_name(uint32_t k)
{
    if (k < NTRK && ambient_layers[k].sound == AS_TIDE && ambient_layers[k].timbre)
        return ambient_layers[k].timbre == 1 ? "T.WARM" : "T.SILK";
    return ambient_layer_name(k);
}
static const char *ambient_scene_name(void)
{
    if (pulse_active(TSEL)) return pulse_timbre_names[ambient_layer(TSEL)->timbre];
    if (ambient_home_active(TSEL) && ambient_layer(TSEL)->sound == AS_TIDE)
        return tide_timbre_names[ambient_layer(TSEL)->timbre];
    return ambient_layer_name(song.sel);
}
static const char *ambient_layer_speed_name(const track_t *t) { return pulse_rate_names[ambient_layer(t)->speed]; }
static const char *ambient_pulse_speed_name(void) { return pulse_active(TSEL) ? pulse_rate_names[ambient_layer(TSEL)->speed] : ""; }
static void ambient_pulse_speed_turn(int32_t steps)
{
    ambient_layer_t *s = ambient_layer(TSEL);
    if (!ambient_home_active(TSEL)) return;
    if (s->mode == AM_SEQUENCE) {
        uint32_t i;
        for (i = 0; i < 4; i++) if (TSEL->p[P_SDIV] == pulse_rates[i]) break;
        int32_t index = clamp((i < 4 ? (int32_t)i : 0) + steps, 0, 3);
        TSEL->p[P_SDIV] = pulse_rates[index];
    } else if (s->mode == AM_SCATTER) {
        s->speed = (uint8_t)clamp((int32_t)s->speed + steps, 0, 3);
        if (steps) s->speed_set = 1;
        TSEL->p[P_ARATE] = pulse_rates[s->speed];
    }
}



static int32_t ambient_level_start(const track_t *t, int32_t target, int32_t *end)
{
    uint32_t k;
    int32_t old, d;
    if (!ambient_home_active(t)) { *end = target; return target; }
    k = (uint32_t)(t - trk);
    old = ambient_level[k]; d = target - old;
    ambient_level[k] = old + (d / 4 ? d / 4 : d);
    *end = ambient_level[k];
    return old;
}

static int ambient_home_keys_held(void)
{
    return ((fm1_in.buttons >> panel.btn[B_HOME]) & 1u) && ui.home &&
           !ui.menu && !ui.confirm && ambient_home_active(TSEL);
}

static __attribute__((noinline)) int ambient_home_key(uint32_t k)
{
    if (!ambient_home_keys_held()) return 0;
    if (key_black(k) && key_place(k) < NTRK) {
        uint32_t layer = key_place(k);
        ambient_muted ^= (uint8_t)(1u << layer);
        if ((ambient_muted >> layer) & 1u) {
            uint32_t i, sounding = (trk[layer].tail && trk[layer].p[P_DIST]) || slicer_busy(&trk[layer]);
            for (i = 0; i < NVOICE; i++) sounding |= trk[layer].v[i].active;
            /* An idle layer has nothing to fade. Its first future note must
             * already be muted, rather than leaking the start of its attack. */
            if (!sounding) pf.mg[layer] = 0;
        }
    }
    return 1;
}

static uint32_t ambient_mute_leds(uint32_t *br)
{
    uint32_t k, lit = 0, available = 0;
    for (k = 0; k < 27u; k++) if (key_black(k) && key_place(k) < NTRK) {
        available |= 1u << k;
        if ((ambient_muted >> key_place(k)) & 1u) lit |= 1u << k;
    }
    if (br) *br = available & ~lit;
    return lit;
}

static void cloud_density_apply(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    /* Rate compensates longer grains: sparse fragments through a full cloud. */
    t->p[P_E3] = clamp(42 + s->density * 70 / 100 - s->shape * 18 / 100, 0, 127);
}

static int32_t ambient_shape_amount(const track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    if (ambient_grounded_pad(t) || ambient_warm_pad(t)) return s->shape;
    if (pulse_active(t)) return s->shape;
    if (texture_active(t)) return s->shape;
    int32_t v = clamp(t->p[P_E4], AMBIENT_SHAPE_MIN, AMBIENT_SHAPE_MAX);
    return (v - AMBIENT_SHAPE_MIN) * 100 / (AMBIENT_SHAPE_MAX - AMBIENT_SHAPE_MIN);
}

static int32_t ambient_shape_range(void)
{
    return AMBIENT_SHAPE_MAX - AMBIENT_SHAPE_MIN;
}

static void ambient_shape_turn(track_t *t, int32_t steps)
{
    ambient_layer_t *s = ambient_layer(t);
    if (ambient_home_active(t) && ambient_grounded_pad(t)) {
        s->shape = (uint8_t)clamp(s->shape + steps, 0, 100);
        t->p[P_E2] = -16 + ambient_two_stage(s->shape, 16, 28);
        return;
    }
    if (ambient_home_active(t) && ambient_warm_pad(t)) {
        s->shape = (uint8_t)clamp(s->shape + steps, 0, 100);
        if (ambient_fm_pad(t)) {
            int32_t bloom = s->motion <= 25 ? 0 : (s->motion - 25) * 48 / 75;
            t->p[P_E2] = clamp(-48 + ambient_two_stage(s->shape, 38, 111) + bloom, -64, 63);
        }
        else {
            t->p[P_E4] = 32 + ambient_two_stage(s->shape, 36, 76);
            t->p[P_E6] = ambient_two_stage(s->shape, 12, 22);
        }
        return;
    }
    if (pulse_active(t)) {
        s->shape = (uint8_t)clamp(s->shape + steps, 0, 100);
        t->p[P_E2] = pulse_shape_min[s->timbre] + s->shape * pulse_shape_span[s->timbre] / 100;
        t->p[P_E3] = 6 + s->shape * (s->timbre == 2 ? 34 : 22) / 100;
        t->p[P_ATK] = 20 - s->shape * 18 / 100;
        t->p[P_DEC] = 52 + s->shape * 38 / 100;
        t->p[P_SUS] = 0;
        t->p[P_REL] = 24 + s->shape * 34 / 100;
        return;
    }
    if (cloud_active(t)) {
        s->shape = (uint8_t)clamp(s->shape + steps, 0, 100);
        t->p[P_E2] = 48 + s->shape * 64 / 100;
        t->p[P_E7] = 112 - s->shape * 24 / 100;
        cloud_density_apply(t);
        return;
    }
    if (texture_active(t)) {
        s->shape = (uint8_t)clamp(s->shape + steps, 0, 100);
        t->p[P_E1] = (int16_t)(100 - s->shape * 70 / 100);
        t->p[P_E2] = (int16_t)(42 + s->shape * 60 / 100);
        return;
    }
    if (ambient_shape_active(t))
        t->p[P_E4] = (int16_t)clamp(t->p[P_E4] + steps, AMBIENT_SHAPE_MIN, AMBIENT_SHAPE_MAX);
}

static void ambient_motion_apply(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    int32_t a = s->motion;
    if (!ambient_home_active(t))
        return;
    t->p[P_LRATE] = (int16_t)(6 + a * 24 / 100);        /* about 0.07 .. 0.24 Hz */
    t->p[P_LWAVE] = 5;                                  /* smooth, irregular DRIFT */
    if (ambient_grounded_pad(t)) {
        t->p[P_LRATE] = a <= 25 ? 6 + a * 3 / 25 : 9 + (a - 25) * 21 / 75;
        t->p[P_LD_FLT] = a <= 25 ? a * 10 / 25 : 10 + (a - 25) * 14 / 75;
        t->p[P_LD_PIT] = t->p[P_LD_SHP] = t->p[P_LD_AMP] = 0;
        return;
    }
    if (ambient_warm_pad(t)) {
        /* Keep the heard 25% default, then expose much stronger evolution. */
        t->p[P_LRATE] = a <= 25 ? 6 + a * 3 / 25 : 9 + (a - 25) * 29 / 75;
        int32_t middle = ambient_fm_pad(t) ? 1 : 2;
        int32_t top = ambient_fm_pad(t) ? -64 : 40;
        t->p[P_LD_FLT] = a <= 25 ? a * middle / 25 : middle + (a - 25) * (top - middle) / 75;
        t->p[P_LD_PIT] = t->p[P_LD_SHP] = t->p[P_LD_AMP] = 0;
        /* SILK's soft default has little FM to animate. Higher MOTION opens
         * a harmonic bed for the drift; SHAPE still sets its tonal centre. */
        if (ambient_fm_pad(t)) ambient_shape_turn(t, 0);
        return;
    }
    if (pulse_active(t)) {
        /* RAND is sampled per note by the existing matrix; tails retain their
         * own tone/gain. Pan uses the latest hit, within a restrained spread. */
        t->p[P_LD_SHP] = t->p[P_LD_FLT] = t->p[P_LD_PIT] = t->p[P_LD_AMP] = 0;
        t->p[P_M1SRC] = MS_RAND; t->p[P_M1DST] = MD_SHP; t->p[P_M1AMT] = a * 28 / 100;
        t->p[P_M2SRC] = MS_RAND; t->p[P_M2DST] = MD_AMP; t->p[P_M2AMT] = a * 32 / 100;
        t->p[P_M3SRC] = MS_RAND; t->p[P_M3DST] = MD_PAN; t->p[P_M3AMT] = a * 18 / 100;
        t->p[P_M4SRC] = t->p[P_M4DST] = t->p[P_M4AMT] = 0;
        return;
    }
    if (cloud_active(t)) {
        t->p[P_LD_SHP] = a * 38 / 100;
        t->p[P_E5] = a * 55 / 100;
        t->p[P_LD_FLT] = t->p[P_LD_PIT] = t->p[P_LD_AMP] = 0;
        return;
    }
    t->p[P_LD_FLT] = (int16_t)(a <= 50 ? a * 8 / 50 : 8 + (a - 50) * 10 / 50);
    t->p[P_LD_PIT] = 0;                                 /* no seasick pitch drift */
    t->p[P_LD_SHP] = 0;
    t->p[P_LD_AMP] = 0;
}

static void ambient_density_apply(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    int32_t a = s->density;
    if (ambient_home_active(t) && ambient_grounded_pad(t)) {
        t->p[P_CHOR] = t->p[P_E6] = 0;
        return; /* Live harmonic weight is applied before FM6's gain ramp. */
    }
    if (ambient_home_active(t) && ambient_warm_pad(t)) {
        t->p[P_CHOR] = s->space ? ambient_two_stage(a, 16, 96) : 0;
        if (ambient_fm_pad(t)) t->p[P_E6] = ambient_two_stage(a, 12, 72);
        else {
            t->p[P_E2] = ambient_two_stage(a, 56, 64);
            t->p[P_E1] = ambient_two_stage(a, 7, 32);
        }
        return;
    }
    if (pulse_active(t)) {
        /* 0% is occasional (4/128 ticks), 100% plays every tick.
         * Speed is exclusively owned by HOME + SELECT. */
        t->p[P_APROB] = 3 + s->density * 124 / 100;
        t->p[P_E4] = ambient_scatter_active(t) ? 0 : s->density * 12 / 100;
        t->p[P_CHOR] = ambient_scatter_active(t) ? 0 : s->density * 60 / 100;
        t->p[P_AGATE] = 32; /* short, separated plucks */
        return;
    }
    if (cloud_active(t)) { cloud_density_apply(t); return; }
    if (texture_active(t)) {
        /* Upper bound ~32 impulses/sec: sparse crackle, not another wash. */
        t->p[P_E5] = (int16_t)(s->density * 58 / 100);
        return;
    }
    if (!ambient_home_active(t))
        return;
    t->p[P_E2] = (int16_t)(a <= 50 ? a * 64 / 50 : 64);
    t->p[P_E1] = (int16_t)ambient_two_stage(a, 16, 36);
    t->p[P_CHOR] = s->space ? (int16_t)ambient_two_stage(a, 52, 100) : 0;
}

static int32_t ambient_two_stage(int32_t a, int32_t middle, int32_t top)
{
    return a <= 50 ? a * middle / 50 : middle + (a - 50) * (top - middle) / 50;
}

static int32_t ambient_space_delay_stage(int32_t a, int32_t middle, int32_t top)
{
    if (a <= 35)
        return 0;
    return a <= 50 ? (a - 35) * middle / 15 : middle + (a - 50) * (top - middle) / 50;
}

static void ambient_space_apply(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    int32_t a = s->space;
    if (ambient_home_active(t) && s->sound == AS_TIDE)
        t->p[P_CHOR] = a && !ambient_grounded_pad(t) ? ambient_two_stage(s->density, s->timbre ? 16 : 52, s->timbre ? 96 : 100) : 0;
    if (ambient_home_active(t) && ambient_warm_pad(t)) {
        t->p[P_DLY] = ambient_space_delay_stage(a, 16, 110);
        t->p[P_REV] = a <= 35 ? a * 42 / 35 : a <= 50 ? 42 + (a - 35) * 18 / 15 : 60 + (a - 50) * 67 / 50;
        s->dry = a <= 50 ? 32767 : 32767 - (a - 50) * 24575 / 50;
        return;
    }
    if (pulse_active(t)) {
        a = s->space;
        t->p[P_DLY] = ambient_space_delay_stage(a, 45, 105);
        t->p[P_REV] = ambient_two_stage(a, 70, 120);
        s->dry = a <= 50 ? 32767 : 32767 - (a - 50) * 24575 / 50;
        return;
    }
    if (texture_active(t)) {
        a = s->space;
        t->p[P_DLY] = (int16_t)ambient_space_delay_stage(a, 30, 90);
        t->p[P_REV] = (int16_t)ambient_two_stage(a, 82, 127);
        s->dry = (int16_t)(a <= 60 ? 32767 : 32767 - (a - 60) * 14367 / 40);
        return; /* Shared bus settings belong to the scene. */
    }
    if (!ambient_home_active(t))
        return;
    t->p[P_DLY] = (int16_t)ambient_space_delay_stage(a, 42, 110);
    t->p[P_REV] = (int16_t)ambient_two_stage(a, 82, 127);
    s->dry = (int16_t)(a <= 60 ? 32767 :
        32767 - (a - 60) * (32767 - 18400) / 40);       /* full .. about -5 dB */
}

static void ambient_pulse_timbre_apply(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    static const uint8_t waves[] = {0, 3, 6}; /* SAW, DSIN, resonant TRI */
    t->p[P_E0] = waves[s->timbre];
    t->p[P_E1] = 0;
    t->p[P_E4] = t->p[P_E5] = t->p[P_E6] = t->p[P_E7] = 0;
    ambient_shape_turn(t, 0);
}

static __attribute__((noinline)) int ambient_latch_note_held(const track_t *t, uint32_t note)
{
    ambient_layer_t *s = ambient_layer(t);
    if (!ambient_latch_active(t)) return 0;
    for (uint32_t i = 0; i < s->count; i++) if (s->notes[i] == note) return 1;
    return 0;
}

static uint32_t ambient_latch_leds(const track_t *t)
{
    uint32_t k, note, m = 0, seen[4] = {0, 0, 0, 0};
    if (!ambient_latch_active(t))
        return 0;
    for (k = 0; k < 27u; k++) {
        note = kb_map(t, k);
        if (note > 127u || ((seen[note >> 5] >> (note & 31u)) & 1u))
            continue;
        seen[note >> 5] |= 1u << (note & 31u);
        m |= (uint32_t)ambient_latch_note_held(t, note) << k;
    }
    return m;
}

static __attribute__((noinline)) int ambient_texture_toggle(track_t *t, uint32_t note, uint32_t vel)
{
    ambient_layer_t *s = ambient_layer(t);
    uint32_t i;
    if (s->count && s->notes[0] == note) {
        s->count = 0;
        if (!midi_note_held(t, note)) trk_note_off(t, note);
        return -1;
    }
    /* Reuse the sounding voice when neither pitch belongs to external MIDI.
     * voice_start preserves its envelope level/filter state, avoiding an attack
     * from silence and another release-tail allocation on register changes. */
    if (s->count && !midi_note_held(t, s->notes[0]) && !midi_note_held(t, note) &&
        t->engine == t->eng_req && !t->xf_on && !t->p[P_MUTE] && trk_vmode(t) == V_POLY) {
        for (i = 0; i < NVOICE; i++) {
            voice_t *v = &t->v[i];
            if (v->active && v->gate && v->note == s->notes[0]) {
                ambient_latch_midi_off(t);
                s->notes[0] = (uint8_t)note;
                mod_note(t, note, vel);
                voice_start(t, v, note, vel, t->p[P_GLIDE] != 0);
                return 1;
            }
        }
    }
    /* Release old register, preserving its tail and independent MIDI owner. */
    ambient_latch_clear(t);
    s->notes[0] = (uint8_t)note;
    s->count = 1;
    if (!midi_note_held(t, note)) trk_note_on(t, note, vel);
    return 1;
}

static __attribute__((noinline)) int ambient_pulse_toggle(track_t *t, uint32_t note)
{
    ambient_layer_t *s = ambient_layer(t);
    uint32_t i;
    for (i = 0; i < s->count; i++) if (s->notes[i] == note) {
        s->notes[i] = s->notes[--s->count];
        if (!midi_note_held(t, note)) arp_remove(t, note);
        if (!t->nheld && arp_sounding(t)) arp_off(t);
        return -1;
    }
    if (s->count == 4u) return 0;
    s->notes[s->count++] = note;
    if (!midi_note_held(t, note)) arp_add(t, note);
    return 1;
}

static __attribute__((noinline)) int ambient_latch_toggle(track_t *t, uint32_t note, uint32_t vel)
{
    ambient_layer_t *s = ambient_layer(t);
    uint32_t i;
    if (!ambient_latch_active(t))
        return 0;
    if (ambient_scatter_active(t)) return ambient_pulse_toggle(t, note);
    if (s->mode == AM_SEQUENCE) return 0;
    if (texture_active(t) && !cloud_active(t)) return ambient_texture_toggle(t, note, vel);
    for (i = 0; i < s->count; i++)
        if (s->notes[i] == note) {
            while (++i < s->count)
                s->notes[i - 1u] = s->notes[i];
            s->count--;
            if (!midi_note_held(t, note))
                trk_note_off(t, note);
            return -1;
        }
    if (s->count >= (cloud_active(t) ? 3u : AMBIENT_LATCH_MAX))
        return 0;
    s->notes[s->count++] = (uint8_t)note;
    if (!midi_note_held(t, note))
        trk_note_on(t, note, vel);
    return 1;
}

static int32_t ambient_motion_amount(const track_t *t)
{ return ambient_home_active(t) ? ambient_layer(t)->motion : 0; }
static int32_t ambient_motion_range(void) { return 100; }
static void ambient_motion_turn(track_t *t, int32_t steps)
{
    ambient_layer_t *s = ambient_layer(t);
    if (!ambient_home_active(t)) return;
    s->motion = (uint8_t)clamp((int32_t)s->motion + steps, 0, 100);
    ambient_motion_apply(t);
}

static int32_t ambient_density_amount(const track_t *t)
{ return ambient_home_active(t) ? ambient_layer(t)->density : 0; }
static int32_t ambient_density_range(void) { return 100; }
static void ambient_density_turn(track_t *t, int32_t steps)
{
    ambient_layer_t *s = ambient_layer(t);
    if (!ambient_home_active(t)) return;
    s->density = (uint8_t)clamp((int32_t)s->density + steps, 0, 100);
    ambient_density_apply(t);
}

static int32_t ambient_space_amount(const track_t *t)
{ return ambient_home_active(t) ? ambient_layer(t)->space : 0; }
static int32_t ambient_space_range(void) { return 100; }
static void ambient_space_turn(track_t *t, int32_t steps)
{
    ambient_layer_t *s = ambient_layer(t);
    if (!ambient_home_active(t)) return;
    s->space = (uint8_t)clamp((int32_t)s->space + steps, 0, 100);
    ambient_space_apply(t);
}

static int32_t ambient_space_dry_gain(const track_t *t)
{ return ambient_home_active(t) ? ambient_layer(t)->dry : 32767; }
static void ambient_latch_midi_off(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    uint32_t i, mc = trk_midi_ch((uint32_t)(t - trk));
    for (i = 0; i < s->count; i++)
        midi_out_event(0x08u | (0x80u | mc) << 8 | (uint32_t)s->notes[i] << 16);
}
static __attribute__((noinline)) void ambient_latch_forget(track_t *t)
{
    ambient_latch_midi_off(t);
    ambient_layer(t)->count = ambient_layer(t)->arp_gate = 0;
}
static void ambient_latch_clear(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    ambient_latch_midi_off(t);
    while (s->count) {
        uint32_t note = s->notes[--s->count];
        if (!midi_note_held(t, note)) {
            if (ambient_scatter_active(t)) arp_remove(t, note);
            else trk_note_off(t, note);
        }
    }
    if (ambient_scatter_active(t) && arp_sounding(t)) arp_off(t);
}
static void ambient_latch_enable(int enabled)
{
    if (!enabled) for (uint32_t i = 0; i < NTRK; i++) ambient_latch_clear(&trk[i]);
    ambient_latch_enabled = (uint8_t)(enabled != 0);
}
/* Used only by deterministic render fixtures to vary a PULSE timbre in place. */
static void ambient_pulse_presets_turn(int32_t steps)
{
    track_t *t = &trk[2];
    uint32_t f = motion_guard();
    ambient_layer(t)->timbre = (uint8_t)((ambient_layer(t)->timbre + (steps > 0 ? 1 : 2)) % 3);
    ambient_pulse_timbre_apply(t);
    motion_unguard(f);
}
/* Configure only playback ownership/timing; never replace the engine or pattern. */
static void ambient_mode_apply(track_t *t)
{
    ambient_layer_t *s = ambient_layer(t);
    t->p[P_AMODE] = s->mode == AM_SCATTER ? 4 : 0;
    t->p[P_AHOLD] = 0; t->p[P_AOCT] = 1; t->p[P_ASWING] = 0;
    t->p[P_ARATE] = pulse_rates[s->speed]; t->p[P_AGATE] = 32;
    if (!pulse_active(t)) {
        t->p[P_APROB] = 127;
        t->p[P_ATK] = s->mode == AM_HOLD ? 88 : 24;
        t->p[P_REL] = s->mode == AM_HOLD ? (s->sound == AS_TIDE ? 112 : 96) : 48;
    } else ambient_density_apply(t);
    if (ambient_grounded_pad(t)) {
        uint8_t patch[FP_SIZE + 1u];
        memcpy(patch, fm6_patch[t - trk], sizeof patch);
        for (uint32_t op = 0; op < 6; op++) {
            patch[op * FP_OP + FP_R1] = s->mode == AM_HOLD ? (op == 4 ? 42 : 50) : 85;
            patch[op * FP_OP + FP_R1 + 3] = s->mode == AM_HOLD ? 45 : 65;
        }
        fm6_set_patch((uint32_t)(t - trk), patch); fm6_adopt((uint32_t)(t - trk));
    }
    if (ambient_warm_pad(t)) {
        if (ambient_fm_pad(t)) {
            /* FM6 owns its envelopes; track ADSR cannot soften its onset. */
            uint8_t patch[FP_SIZE + 1u];
            memcpy(patch, fm6_patch[t - trk], sizeof patch);
            for (uint32_t op = 0; op < 6; op++) {
                patch[op * FP_OP + FP_R1] = s->mode == AM_HOLD ? (op & 1 ? 44 : 38) : 85;
                patch[op * FP_OP + FP_R1 + 3] = s->mode == AM_HOLD ? 44 : 65;
            }
            fm6_set_patch((uint32_t)(t - trk), patch); fm6_adopt((uint32_t)(t - trk));
        } else {
            t->p[P_ATK] = s->mode == AM_HOLD ? 80 : 32;
            t->p[P_REL] = s->mode == AM_HOLD ? 100 : 60;
        }
    }
    if (cloud_active(t) && s->mode != AM_HOLD) {
        static const uint8_t attack[] = {24, 72, 80};
        static const uint8_t release[] = {48, 80, 88};
        static const uint8_t arp_gate[] = {32, 112, 127};
        static const uint8_t seq_gate[] = {64, 112, 127};
        uint32_t length = s->cloud_length;
        if (length > 2u) length = 1;
        /* The first untouched CLOUD clock starts slowly. Subsequent explicit
         * speed choices survive sound/mode changes and length edits. */
        if (s->mode == AM_SCATTER && !s->speed_set) {
            s->speed = 0; t->p[P_ARATE] = pulse_rates[0];
        }
        t->p[P_ATK] = attack[length]; t->p[P_REL] = release[length];
        t->p[P_AGATE] = arp_gate[length];
        if (s->mode == AM_SEQUENCE) t->p[P_SGATE] = seq_gate[length];
    }
}
static const char *ambient_cloud_length_name(const track_t *t)
{
    static const char *const names[] = {"SHORT", "FLOW", "LONG"};
    uint32_t length = ambient_layer(t)->cloud_length;
    return names[length < 3u ? length : 1u];
}
static void ambient_cloud_length_turn(track_t *t)
{
    uint32_t f;
    if (!cloud_active(t)) return;
    if (ambient_layer(t)->mode == AM_HOLD) { ui_message("HOLD IS SUSTAINED"); return; }
    f = motion_guard();
    ambient_layer(t)->cloud_length = (uint8_t)((ambient_layer(t)->cloud_length + 1u) % 3u);
    ambient_mode_apply(t);
    motion_unguard(f);
    ui_message(ambient_cloud_length_name(t)); ui.force = 1;
}
static void ambient_mode_set(track_t *t, uint32_t mode)
{
    ambient_layer_t *s = ambient_layer(t);
    uint32_t k = (uint32_t)(t - trk), f;
    if (!ambient_home_active(t) || mode > AM_SEQUENCE || s->mode == mode) return;
    if (chain_busy()) { ui_message("STOP SONG FIRST"); return; }
    f = motion_guard();
    ambient_latch_clear(t);
    seq_release(t); trk_all_off(t);
    t->nheld = t->arp_phys = t->arp_note = t->xp_n = s->arp_gate = 0;
    /* A mode change is a local input reset, including external MIDI ownership. */
    midi_forget_track(k);
    s->mode = (uint8_t)mode;
    if (mode == AM_SEQUENCE && seq_is_empty(t) && t->p[P_SLEN] == TP[P_SLEN].def) {
        t->p[P_SLEN] = 8; t->p[P_SDIV] = 6; /* empty-track starting point: slow eight-step phrase */
    }
    t->seq_idx = (uint16_t)(t->p[P_SLEN] - 1);
    t->seq_pos = 0x7FFFFFFF; t->rat_left = 0;
    t->arp_pos = 0xFFFFFFF; t->arp_idx = 0xFFFFFFFFu;
    ambient_mode_apply(t);
    motion_unguard(f);
    ui_message(ambient_mode_name(t)); ui.force = 1;
}
/* A deliberate sound change starts silent. Only this layer is reset. */
/* Original dry FM audition, retained exactly at macro defaults. */
static void ambient_tide_patch(track_t *t)
{
    uint8_t patch[FP_SIZE+1]; fm6_unpack(FM6_INIT,patch);
    patch[FP_ALG]=4; patch[FP_FB]=0; patch[FP_OKS]=1;
    patch[FP_LPMD]=patch[FP_LAMD]=0;
    for(unsigned op=0;op<6;op++) {
        uint8_t *p=patch+op*FP_OP;
        p[FP_OL]=0; p[FP_MODE]=0; p[FP_FC]=1; p[FP_FF]=0; p[FP_DET]=7;
        p[FP_KVS]=p[FP_RS]=p[FP_AMS]=p[FP_LD]=p[FP_RD]=0;
        p[FP_R1]=50; p[FP_R1+1]=99; p[FP_R1+2]=99; p[FP_R1+3]=45;
        p[FP_L1]=p[FP_L1+1]=p[FP_L1+2]=99; p[FP_L1+3]=0;
    }
    /* Algorithm 5: one 1:1 FM pair and a quiet, unmodulated 2nd harmonic.
     * No extra fundamental carriers, spread, feedback or pitch modulation. */
    patch[5*FP_OP+FP_OL]=99;                 /* OP1 carrier, stable sustain */
    patch[4*FP_OP+FP_OL]=66;                 /* OP2 modulator, ~0.115 turns at EG level 99 */
    patch[4*FP_OP+FP_R1]=42;                 /* harmonic bloom after onset */
    patch[3*FP_OP+FP_OL]=75;
    patch[3*FP_OP+FP_FC]=2;
    memcpy(patch+FP_NAME,"TIDE BODY ",10);
    fm6_set_patch((uint32_t)(t-trk),patch); fm6_adopt((uint32_t)(t-trk));
    for(unsigned p=P_E0;p<P_E7;p++)t->p[p]=0;
    t->p[P_LRATE]=9; t->p[P_LWAVE]=5;
    t->p[P_LD_FLT]=10;                        /* existing DRIFT -> FM index */
    t->p[P_ED_FLT]=t->p[P_LD_PIT]=t->p[P_LD_SHP]=t->p[P_LD_AMP]=0;
}

static void ambient_sound_apply(track_t *t, uint32_t sound, uint32_t timbre)
{
    ambient_layer_t *s = ambient_layer(t);
    uint32_t k = (uint32_t)(t - trk), muted = ambient_muted & (1u << k);
    int16_t level = t->p[P_LEVEL];
    uint8_t mode = s->mode, speed = s->speed, speed_set = s->speed_set, length = s->cloud_length;
    uint32_t f = motion_guard();
    static const uint8_t engines[] = {0, 0, 11, 2, 8};
    ambient_latch_clear(t);
    trk_all_off(t);
    t->nheld = t->arp_phys = t->arp_note = t->xp_n = 0;
    /* These engines have full eight-parameter factory presets. Use the
     * existing preset loader under our outer IRQ guard, rather than the
     * general engine helper which unconditionally enables IRQs on return. */
    load_begin(t, UNDO_SOUND);
    t->eng_req = sound == AS_TIDE && timbre != 1 ? ENGI_FM6 : engines[sound];
    apply_preset_to(t, t->eng_req == ENGI_FM6 ? 4 : sound == AS_TIDE ? AMBIENT_ANALOG_PRESET : 0);
    load_end(t);
    memset(s, 0, sizeof *s);
    s->sound = (uint8_t)sound; s->timbre = (uint8_t)timbre; s->mode = mode; s->speed = speed;
    s->speed_set = speed_set; s->cloud_length = length;
    s->shape = 35; s->motion = 25; s->density = sound == AS_TEXTURE ? 25 : 50; s->space = 50;
    if (ambient_grounded_pad(t)) s->shape = 50;
    if (ambient_warm_pad(t)) { s->shape = 50; s->space = 35; }
    t->p[P_LEVEL] = level; t->p[P_MUTE] = 0; t->p[P_AMODE] = 0; t->p[P_CHOR] = 0;
    ambient_muted = (uint8_t)((ambient_muted & ~(1u << k)) | muted);
    t->p[P_ATK] = 88; t->p[P_DEC] = 96; t->p[P_SUS] = 120; t->p[P_REL] = 96;
    if (sound == AS_TIDE) {
        if (timbre == 2) {
            uint8_t patch[FP_SIZE + 1u];
            memcpy(patch, fm6_patch[k], sizeof patch);
            patch[FP_LPMD] = 0; /* richness without pitch wobble */
            /* Algorithm 5 has three carrier/modulator pairs. Keep carrier
             * levels even through the sustain, avoiding a loud opening bump. */
            for (uint32_t op = 1; op < 6; op += 2) {
                for (uint32_t stage = 0; stage < 3; stage++) patch[op * FP_OP + FP_L1 + stage] = 94;
                patch[op * FP_OP + FP_OL] += 4;
            }
            memcpy(patch + FP_NAME, "SILK PAD  ", 10);
            fm6_set_patch(k, patch); fm6_adopt(k);
            t->p[P_E1] = -1; t->p[P_E3] = t->p[P_E4] = t->p[P_E5] = 0;
            t->p[P_ED_FLT] = 0;
        } else if (timbre == 1) {
            t->p[P_E3] = 0; t->p[P_E5] = 6; t->p[P_E6] = 12; t->p[P_E7] = 48;
            t->p[P_ED_FLT] = 0; t->p[P_SUS] = 124;
        } else {
            ambient_tide_patch(t);
            t->p[P_REL] = 112;
        }
    } else if (sound == AS_TEXTURE) {
        t->p[P_E0] = 0; t->p[P_E3] = 18; t->p[P_E4] = 24;
        t->p[P_E6] = t->p[P_E7] = 0;
    } else if (sound == AS_CLOUD) {
        t->p[P_E0] = 2; t->p[P_E1] = 50; t->p[P_E4] = t->p[P_E6] = 0;
    } else {
        t->p[P_AMODE] = 4; t->p[P_AHOLD] = 0; t->p[P_AOCT] = 1;
        t->p[P_ASWING] = 0; t->p[P_ARATE] = pulse_rates[s->speed];
        ambient_pulse_timbre_apply(t);
    }
    ambient_shape_turn(t, 0); ambient_motion_apply(t);
    ambient_density_apply(t); ambient_space_apply(t);
    ambient_mode_apply(t);
    motion_unguard(f);
    ui.force = 1;
}
/* Every HOME has the same curated list. PULSE variants share clock behavior. */
static void ambient_presets_turn(int32_t steps)
{
    ambient_layer_t *s = ambient_layer(TSEL);
    static const uint8_t sounds[] = {AS_TIDE, AS_TEXTURE, AS_CLOUD, AS_PULSE, AS_PULSE, AS_PULSE, AS_TIDE, AS_TIDE};
    static const uint8_t timbres[] = {0, 0, 0, 0, 1, 2, 1, 2};
    int32_t index = s->sound == AS_TIDE ? (s->timbre ? 5 + s->timbre : 0) : s->sound == AS_TEXTURE ? 1 : s->sound == AS_CLOUD ? 2 : 3 + s->timbre;
    if (!steps) return;
    index = (index + steps % 8 + 8) % 8;
    ambient_sound_apply(TSEL, sounds[index], timbres[index]);
    ui_message(ambient_scene_name());
}
static void ambient_boot_apply(void)
{
    static const uint8_t sounds[] = {AS_TIDE, AS_TEXTURE, AS_PULSE, AS_CLOUD};
    /* User-reviewed layered balance: keep TIDE's patch/macros unchanged and
     * leave room for PULSE over a sustained chord. */
    static const uint8_t levels[] = {82, 80, 94, 80};
    ambient_latch_enable(0);
    ambient_latch_enabled = 1;
    ambient_muted = ambient_mix = 0;
    song.g[G_DTIME] = 1; song.g[G_DFDBK] = 54; song.g[G_DCOLOR] = 62;
    song.g[G_DMIX] = 82; song.g[G_RSIZE] = 108; song.g[G_RDAMP] = 76;
    song.g[G_CRATE] = 32; song.g[G_CDEPTH] = 52;
    for (uint32_t i = 0; i < NTRK; i++) {
        ambient_layers[i].mode = i == 2 ? AM_SCATTER : AM_HOLD;
        ambient_layers[i].speed = i == 3 ? 0 : 2;
        ambient_layers[i].speed_set = 0; ambient_layers[i].cloud_length = 1;
        ambient_sound_apply(&trk[i], sounds[i], 0);
        trk[i].engine = trk[i].eng_req;
        trk[i].p[P_LEVEL] = levels[i];
        ambient_level[i] = LEVEL_Q12[levels[i]];
        track_defaults_steps(&trk[i]);
        pat_sig[i] = steps_sig(&trk[i]); pat_last[i] = 0;
    }
    song.sel = song.playing = song.rec = transport_req = 0;
}
#endif
