/* SPDX-License-Identifier: GPL-3.0-only */
#include "../web/emu/felucca_web.c"
static FILE *recording;
static void take(unsigned milliseconds)
{
    unsigned frames=milliseconds*441u/10u;
    for(unsigned f=0;f<frames;f+=128) {
        unsigned n=frames-f<128?frames-f:128;
        web_render(n);
        if(recording)for(unsigned i=0;i<n;i++) {
            float stereo[]={out_l[i],out_r[i]};fwrite(stereo,sizeof stereo,1,recording);
        }
    }
}
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    web_nor_erase();web_boot();take(1000);
    web_enc(EN_K4,-100);take(40); /* dry through the real knob */
    web_keys((1u<<7)|(1u<<11)|(1u<<14)|(1u<<18));take(60);web_keys(0);take(60);
    recording=fopen(argv[1],"wb");if(!recording)return 2;
    take(20000);
    web_enc(EN_K2,100);take(10000);
    web_enc(EN_K2,-100);take(10000);
    fclose(recording);return 0;
}
