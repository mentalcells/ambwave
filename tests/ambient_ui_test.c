/* SPDX-License-Identifier: GPL-3.0-only */
/* Focused performance HOME/MIX lint, using the full UI renderer's hooks. */
#define AMBIENT_POC 1
#define UI_RENDER_MAIN ambient_ui_reference_main
#include "ui_render.c"
#undef UI_RENDER_MAIN

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "build/ui_ambient";
    char path[600], name[96]; uint32_t screens = 0;
    snprintf(path,sizeof path,"%s/report.txt",out);rep=fopen(path,"w");
    if(!rep)return 1;
    snprintf(path,sizeof path,"%s/align.txt",out);al_rep=fopen(path,"w");
    snprintf(path,sizeof path,"%s/text_audit.tsv",out);audf=fopen(path,"w");
    for(uint32_t p=0;p<NPALETTES;p++) for(uint32_t style=ST_FLAT;style<=ST_LINE;style++)
    for(uint32_t large=0;large<2;large++) for(uint32_t selected=0;selected<4;selected++)
    for(uint32_t timbre=0;timbre<3;timbre++) for(uint32_t muted=0;muted<16;muted++)
    for(uint32_t playback=0;playback<3;playback++) for(uint32_t rotation=0;rotation<4;rotation++) for(uint32_t mix=0;mix<2;mix++) {
        ui_power_on();song.sel=selected;
        static const uint8_t sounds[]={AS_TIDE,AS_TEXTURE,AS_PULSE,AS_CLOUD};
        for(uint32_t k=0;k<NTRK;k++) {
            ambient_sound_apply(&trk[k],sounds[(k+rotation)%4],timbre);
            ambient_layers[k].count=1; ambient_layers[k].speed=3; ambient_layers[k].mode=(playback+k)%3;
            ambient_layers[k].cloud_length=timbre;
        }
        ambient_muted=muted;ambient_mix=mix;
        trk[0].p[P_LEVEL]=trk[1].p[P_LEVEL]=trk[2].p[P_LEVEL]=127;
        ambient_layers[0].count=4;ambient_layers[1].count=1;ambient_layers[2].count=4;ambient_layers[3].count=1;song.playing=1;
        for(uint32_t k=0;k<NTRK;k++)if(cloud_active(&trk[k])&&ambient_layers[k].count>3)ambient_layers[k].count=3;
        pal(p);ui_style=style;style_apply();large_on=large;
        if(large)ui_prefs|=PREF_LARGE;else ui_prefs&=~PREF_LARGE;
        snprintf(name,sizeof name,"HOME %u timbre %u mute %u mix %u palette %u style %u large %u",selected,timbre,muted,mix,p,style,large);
        cur_name=name;draw(-1);lint();mono_check();screens++;
        if(rotation==0&&selected==3&&p==UI_GREY_INDEX&&style==ST_FLAT&&!large&&timbre==0&&muted==0)
            write_ppm(out,"GREY",mix?"cloud_mix":"cloud_home");
        if(rotation==0&&p==UI_GREY_INDEX&&style==ST_FLAT&&!large&&selected==2&&timbre==0&&muted==5) {
            write_ppm(out,"GREY",mix?"pulse_mix_mutes":"pulse_home_mutes");
        }
    }
    al_table();fprintf(rep,"%u focused HOME/MIX screens; %u lint findings, %u alignment failures\n",screens,nfind,al_fail);
    fclose(rep);if(al_rep)fclose(al_rep);if(audf)fclose(audf);
    printf("ambient UI: %u screens, %u lint findings, %u alignment failures\n",screens,nfind,al_fail);
    return nfind||al_fail||al_lost ? 1:0;
}
