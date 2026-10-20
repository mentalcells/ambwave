/* SPDX-License-Identifier: GPL-3.0-only */
/* Three independent smooth random walks, evaluated at block rate. No audio
 * buffers, global RNG use, note-clock dependency or stored patch mutation. */
#if AMBIENT_POC
typedef struct {
    uint32_t elapsed, duration;
    int32_t from, to, value;
} drone_walk_t;
typedef struct {
    drone_walk_t walk[3];
    uint32_t seed;
    int32_t control[3]; /* Q16: smoothed Shape, Motion, Density */
    int32_t cut, gain, fine, detune;
    uint8_t on;
} drone_state_t;
static drone_state_t drones[NTRK];
static uint32_t drone_random(drone_state_t *d)
{
    d->seed ^= d->seed << 13; d->seed ^= d->seed >> 17; d->seed ^= d->seed << 5;
    return d->seed;
}
static void drone_reset(track_t *t)
{
    drone_state_t *d = &drones[t - trk]; int32_t c[3] = {50, 25, 50};
    memset(d, 0, sizeof *d);
    d->seed = 0x6d2b79f5u ^ ((uint32_t)(t - trk) + 1u) * 0x9e3779b9u;
    ambient_drone_controls(t, c);
    for (uint32_t j = 0; j < 3; j++) d->control[j] = c[j] * 65536;
    d->gain = 32767;
}
static __attribute__((noinline)) void drone_tick(track_t *t, uint32_t n)
{
    drone_state_t *d = &drones[t - trk]; int32_t c[3];
    d->on = ambient_drone_controls(t, c) && t->engine == 0 && !t->xf_on;
    if (!d->on) return;
    if (!d->seed) drone_reset(t);
    for (uint32_t j = 0; j < 3; j++) {
        int32_t diff = c[j] * 65536 - d->control[j];
        d->control[j] += diff / 64 + (diff && diff / 64 == 0 ? (diff > 0 ? 1 : -1) : 0);
        drone_walk_t *w = &d->walk[j];
        if (!w->duration || w->elapsed >= w->duration) {
            w->from = w->value;
            w->to = (int32_t)(drone_random(d) >> 16) - 32768;
            /* Unequal intervals, 20..120 seconds. Faster settings shorten
             * subsequent journeys; macro depth responds immediately/smoothly. */
            uint32_t seconds = 20u + drone_random(d) % (101u - (uint32_t)c[1] * 70u / 100u);
            w->duration = seconds * FS; w->elapsed = 0;
        }
        w->elapsed += n;
        int32_t x = (int32_t)(w->elapsed / (w->duration >> 15));
        x = clamp(x, 0, 32767);
        int32_t smooth = ((x * x) >> 15) * (98304 - 2 * x) >> 15;
        w->value = w->from + ((w->to - w->from) * smooth >> 15);
    }
    int32_t motion = d->control[1] >> 8; /* Q8 percent */
    d->cut = ((d->control[0] >> 8) - 50 * 256) * 48 / 100 +
        (d->walk[0].value * motion / 25600) * 12 / 128;
    d->gain = 32767 - ((d->walk[1].value + 32768) * (motion / 16) / 25600);
    d->fine = d->walk[2].value * motion / 25600 / 560; /* <= ~1.5 cents, Q16 factor */
    d->detune = (((d->control[2] >> 8) * 36 / 100) - 20 * 256) * 38 / 256;
}
static uint32_t drone_detune(track_t *t, uint32_t inc)
{
    drone_state_t *d = &drones[t - trk];
    return d->on ? inc + (uint32_t)((int32_t)(inc >> 16) * d->detune) : inc;
}
static void drone_voice(track_t *t, voice_t *v, vmod_t *m)
{
    const drone_state_t *d = &drones[t - trk];
    if (!d->on) return;
    m->cutoff += d->cut;
    m->inc += (uint32_t)((int32_t)(m->inc >> 16) * d->fine);
    m->amp1 = mulq15(m->amp1, d->gain);
    v->env_out = m->amp1; /* existing per-sample gain ramp starts here next block */
}
#endif
