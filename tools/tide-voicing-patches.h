/* SPDX-License-Identifier: GPL-3.0-only */
/* Listening-only P0b recipes. Never included by firmware or factory presets.
 * B uses the same three active operators; C adds one quiet carrier to B. */
static void tide_voicing_setup(track_t *t)
{
    const char *setting=getenv("TIDE_VOICING");
    unsigned variant=setting?(unsigned)atoi(setting):0;
    if(!variant)return; /* A is the actual current patch, without reconstruction. */
    if(variant>3) {fprintf(stderr,"TIDE_VOICING must be 0, 1, 2 or 3\n");exit(2);}
    uint8_t patch[FP_SIZE+1];
    unsigned tr=(unsigned)(t-trk);
    memcpy(patch,fm6_patch[tr],sizeof patch);
    /* Add an odd-harmonic body alongside the retained fundamental/second
     * harmonic, rather than repeating the earlier second-harmonic gain test. */
    if(variant!=3) { /* D retains every part of A's original warm voicing. */
        patch[4*FP_OP+FP_FC]=2;
        patch[4*FP_OP+FP_OL]=72;
        patch[3*FP_OP+FP_OL]=78;
    }
    if(variant==2 || variant==3) {
        patch[1*FP_OP+FP_OL]=82; /* quiet third pair's unmodulated fundamental */
        patch[1*FP_OP+FP_FC]=1;
        patch[1*FP_OP+FP_DET]=8; /* one native detune step; no pitch modulation */
    }
    fm6_set_patch(tr,patch);fm6_adopt(tr);
}
