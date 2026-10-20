/* SPDX-License-Identifier: GPL-3.0-only */
#define AMBIENT_REFERENCE_MAIN ambient_reference_main
#include "ambient_test.c"
#undef AMBIENT_REFERENCE_MAIN
#include <time.h>
#include <limits.h>
static void drone_blocks(uint32_t blocks)
{ int32_t out[CTL * 2]; while (blocks--) mix_block(out, CTL); }
static void drone_setup(uint32_t k)
{
    ambient_audio_reset(); song.sel = k;
    ambient_patch_apply(&trk[k], AS_DRONE, 0); drone_blocks(100);
}
int main(void)
{
    int bad = 0;
    drone_setup(0);
    bad += ambient_check("Drone profile preserves factory oscillator/filter/envelope source",
        trk[0].p[P_E0] == 0 && trk[0].p[P_E1] == 20 && trk[0].p[P_E2] == 64 && trk[0].p[P_E3] == 4 &&
        trk[0].p[P_E4] == 60 && trk[0].p[P_E5] == 10 && trk[0].p[P_ATK] == 80 && trk[0].p[P_SUS] == 110 && trk[0].p[P_REL] == 95);
    for (uint32_t k = 0; k < NTRK; k++) {
        drone_setup(k); track_t *t = &trk[k]; ambient_layer_t *s = ambient_layer(t);
        bad += ambient_check("Drone loads on any track with one sustained Analog profile",
            ambient_drone_active(t) && !strcmp(ambient_patch_name(), "AN SOFT PAD") && s->mode == AM_HOLD);
        key_down(7); key_down(11); key_up(7); key_up(11); drone_blocks(100);
        uint32_t n0 = s->notes[0], n1 = s->notes[1];
        bad += ambient_check("physical chord survives release without PLAY", s->count == 2 && gate_note(t,n0) && gate_note(t,n1) && !song.playing);
        song.sel = (k + 1) % NTRK; ambient_tap_key(14); drone_blocks(100);
        bad += ambient_check("playing another track leaves Drone harmony intact", s->count == 2 && gate_note(t,n0) && gate_note(t,n1));
        song.sel = k;
        key_down(14); key_down(18); key_up(14); key_up(18); drone_blocks(100);
        bad += ambient_check("next gesture replaces rather than accumulates chord",
            s->count == 2 && s->notes[0] != n0 && !gate_note(t,n0));
        uint8_t notes[4]; memcpy(notes,s->notes,4);
        ambient_muted |= 1u << k; drone_blocks(100);
        ambient_muted &= ~(1u << k); drone_blocks(100);
        bad += ambient_check("mute/unmute preserves Drone latch", s->count == 2 && !memcmp(notes,s->notes,4));
        ambient_mode_set(t, AM_SCATTER);
        bad += ambient_check("Drone cannot become transport-dependent SCATTER", s->mode == AM_HOLD && s->count == 2);
        ambient_shape_turn(t, 20); ambient_motion_turn(t, 30); ambient_density_turn(t, 30);
        ambient_space_turn(t, -100); drone_blocks(600);
        bad += ambient_check("all macros act without changing harmony or dropping gates",
            s->shape == 70 && s->motion == 55 && s->density == 80 && !t->p[P_DLY] && !t->p[P_REV] && !t->p[P_CHOR] &&
            s->count == 2 && gate_note(t,s->notes[0]) && gate_note(t,s->notes[1]) && drones[k].cut > 0 && drones[k].detune > 0);
        ambient_patch_apply(t, AS_TIDE, 0); drone_blocks(100);
        bad += ambient_check("role change releases Drone notes and disables its DSP", !s->count && !drones[k].on);
    }
    drone_setup(0); usb.config=1; mo_r=mo_w=0;
    midi_note_event(0,48,100); midi_note_event(0,48,0);
    midi_note_event(0,50,100); midi_note_event(0,50,0);
    ambient_latch_clear(&trk[0]);
    bad += ambient_check("MIDI-only Drone replacement/clear emits no unmatched external note-offs", mo_r == mo_w);
    ambient_tap_key(7); uint32_t sent=mo_w; ambient_latch_clear(&trk[0]);
    bad += ambient_check("local Drone keys retain balanced MIDI output note-off", sent == 1 && mo_w == 2);
    drone_setup(0); song.g[G_ROUTE] = 1;
    midi_note_event(0,48,100); midi_note_event(0,55,100);
    midi_note_event(0,48,0); midi_note_event(0,55,0); drone_blocks(100);
    bad += ambient_check("MIDI chord latches after note-offs", ambient_layers[0].count == 2 && gate_note(&trk[0],48) && gate_note(&trk[0],55));
    song.sel = 1; midi_note_event(0,65,100); song.sel = 0; midi_note_event(0,65,0);
    bad += ambient_check("MIDI note-off retains original track ownership", !midi_owners[1] && ambient_layers[0].count == 2);
    midi_note_event(0,50,100); midi_note_event(0,57,100);
    midi_note_event(0,50,0); midi_note_event(0,57,0);
    bad += ambient_check("MIDI next chord replaces previous harmony", ambient_layers[0].count == 2 && !gate_note(&trk[0],48) && gate_note(&trk[0],50));
    midi_control(0,123,0); drone_blocks(100);
    bad += ambient_check("MIDI All Notes Off clears latch", !ambient_layers[0].count && !ambient_gated_notes(&trk[0]));
    midi_note_event(0,60,100); midi_note_event(0,60,0); midi_control(0,120,0); drone_blocks(100);
    bad += ambient_check("MIDI All Sound Off clears ownership and voices", !ambient_layers[0].count && !midi_owners[0]);
    drone_setup(0); ambient_tap_key(7);
    btn_down(B_FX); frame(); press(B_OCTDN); btn_up(B_FX); frame(); drone_blocks(100);
    bad += ambient_check("existing FX + OCT-minus gesture clears Drone", !ambient_layers[0].count && !ambient_gated_notes(&trk[0]));
    drone_setup(0); ambient_tap_key(7);
    ambient_presets_turn(1); drone_blocks(10);
    bad += ambient_check("one-sound Drone browsing does not interrupt the latch", ambient_layers[0].count == 1);
    ambient_shape_turn(&trk[0],5); /* end the existing coalesced preset-browse transaction */
    ambient_patch_apply(&trk[0], AS_TIDE, 0); drone_blocks(100);
    undo_swap(); drone_blocks(100);
    bad += ambient_check("sound undo restores Drone profile without stuck notes", ambient_drone_active(&trk[0]) && !ambient_layers[0].count && drones[0].on);
    undo_swap(); drone_blocks(100);
    bad += ambient_check("redo leaves Drone DSP disabled on Tide", ambient_layers[0].role == AS_TIDE && !drones[0].on);
    drone_setup(0); ambient_shape_turn(&trk[0],5); key_down(7);
    ui.home = 0; ambient_mix = 0; presets_turn(1); drone_blocks(100); key_up(7);
    undo_swap(); drone_blocks(100);
    bad += ambient_check("full-UI sound undo discards stale physical-key ownership",
        ambient_drone_active(&trk[0]) && !ambient_layers[0].drone_keys && !ambient_layers[0].count);
    drone_setup(0); ambient_shape_turn(&trk[0],15); ambient_motion_turn(&trk[0],30);
    int16_t saved[P_COUNT]; memcpy(saved,trk[0].p,sizeof saved);
    static project_t project; static project_store_t wire;
    project_capture(&project); int packed=proj_pack(&wire,&project);
    ambient_boot_apply(); int unpacked=proj_unpack(&project,wire.raw,sizeof wire);
    int restored=project_restore_runtime(&project); drone_blocks(100);
    bad += ambient_check("Drone source round-trips existing project format without claiming role recall",
        packed && unpacked && !restored && !ambient_home_active(&trk[0]) &&
        !memcmp(&saved[P_E0],&trk[0].p[P_E0],8*sizeof(int16_t)));
    /* Five minutes of block-clock evolution, without notes, transport or UI.
     * Verify independent movement, bounds, static minimum and patch immutability. */
    for (uint32_t amount = 0; amount <= 100; amount += 25) {
        drone_setup(0); ambient_layers[0].motion = amount; drone_reset(&trk[0]);
        int16_t before[P_COUNT]; memcpy(before,trk[0].p,sizeof before);
        int32_t lo[3] = {INT32_MAX,INT32_MAX,INT32_MAX}, hi[3] = {INT32_MIN,INT32_MIN,INT32_MIN};
        for (uint32_t sample = 0; sample < 300u * FS; sample += CTL) {
            drone_tick(&trk[0],CTL); drone_state_t *d = &drones[0];
            int32_t val[3] = {d->cut,d->gain,d->fine};
            for (uint32_t j=0;j<3;j++) { if(val[j]<lo[j])lo[j]=val[j]; if(val[j]>hi[j])hi[j]=val[j]; }
        }
        printf("drone: Motion %u five-minute cut/gain/fine ranges %d..%d / %d..%d / %d..%d\n",amount,lo[0],hi[0],lo[1],hi[1],lo[2],hi[2]);
        bad += ambient_check("five-minute walks are bounded, independent and zero at Motion minimum",
            amount ? hi[0]>lo[0]+32 && hi[1]>lo[1]+32 && hi[2]>lo[2] && lo[1]>=28671 && lo[2]>=-59 && hi[2]<=59 :
            lo[0]==0 && hi[0]==0 && lo[1]==32767 && hi[1]==32767 && lo[2]==0 && hi[2]==0);
        bad += ambient_check("evolution never mutates stored source parameters", !memcmp(before,trk[0].p,sizeof before));
    }
    /* Four active Drone tracks exercise the highest new control-state count. */
    drone_setup(0);
    for(uint32_t k=0;k<NTRK;k++) { ambient_patch_apply(&trk[k],AS_DRONE,0); }
    drone_blocks(100);
    for(uint32_t k=0;k<NTRK;k++) { song.sel=k; ambient_tap_key(7); }
    clock_t start=clock(); drone_blocks(10u*FS/CTL);
    double cost=(double)(clock()-start)/CLOCKS_PER_SEC/10.;
    printf("drone: four-track host render / real time %.6f; state %zu bytes\n",cost,sizeof drones);
    bad += ambient_check("four active Drone tracks meet host budget", cost < .5);
    printf("drone: %s\n",bad ? "FAILED" : "all passed"); return bad != 0;
}
