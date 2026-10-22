/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments; GHOULBOX changes 2026; DIRGE changes 2026 */
/* TAPE (DIRGE, G_TAPE on FX > TAPE): the whole mix through a worn cassette, before the FX layer and MASTER. GHOULBOX's
 * tape.c (jasonpersinger/ghoulbox-fm1-dungeon-synth, GPL-3.0), its TAPE half as it was; its CRSH left out (Felucca's
 * INSERT CRUSH does that on a track). DIRGE's: the reverb's mono return (wet[]) folded into the stereo mix first, as
 * the master would add it, so the tails wear too. TAPE 0: nothing runs and wet[] is left alone, the mix bit for bit
 * as without it.
 * TAPE (one "age" knob): wow (0.5 Hz, its depth wandering over seconds) and flutter (7 Hz) as a moving read of a short
 *   line (up to ~0.9 % and ~0.15 % of pitch at 127; the depth grows with TAPE squared: the first half of the knob is
 *   subtle), a tanh saturation (drive 1x .. 1.75x, made up: quiet parts pass at unity, the peaks round off), a 12
 *   dB/oct high cut (~14 kHz .. ~3.5 kHz) and hiss (to ~-55 dB). DIRGE: the hiss follows the music (an envelope of
 *   the input, ~0.4 s release), so a stopped FM-1 with TAPE on is silent, not a hissing deck. The line's centre is fixed (TP_C): only the depth
 *   moves with the knob, the pitch does not jump; switching TAPE on starts the line from silence (~3 ms).
 * Audio ISR (fx.c mix_block); its line in the pool: 4 KiB. */
#define TP_LEN 512u                                     /* the wow line, stereo, int32: 4 KiB */
#define TP_MASK (TP_LEN - 1u)
#define TP_C (140 << 8)                                 /* its centre delay, Q8 samples (~3.2 ms) */
#define TP_WOW_INC 1558426u                             /* phase steps per block (FS / CTL a second): 0.5 Hz */
#define TP_FLUT_INC 21818000u                           /* 7 Hz */
static int32_t tp_buf[2][TP_LEN] __attribute__((section(".pool")));
static struct {
    uint32_t w, ph_wow, ph_flut;
    int32_t d_prev, drift, drift_to, rng, hiss_lp[2], lp1[2], lp2[2], env;
    uint8_t on;
} tp;

static __attribute__((noinline)) void tape_block(int32_t *l, int32_t *r, int32_t *wet, uint32_t n)
{
    int32_t t = song.g[G_TAPE], *ch[2] = {l, r};
    uint32_t i, k;
    if (!t) {
        tp.on = 0;
        return;
    }
    if (!tp.on) {                                       /* from silence: no stale line, no old state */
        memset(tp_buf, 0, sizeof tp_buf);
        memset(&tp, 0, sizeof tp);
        tp.rng = 0x2545F491;
        tp.d_prev = TP_C;
        tp.on = 1;
    }
    {
        int32_t pk = 0;
        for (i = 0; i < n; i++) {                       /* the reverb's return into the mix, as the master adds it */
            int32_t a, b;
            l[i] += wet[i];
            r[i] += wet[i];
            wet[i] = 0;
            a = l[i] < 0 ? -l[i] : l[i];
            b = r[i] < 0 ? -r[i] : r[i];
            pk = a > pk ? a : pk;
            pk = b > pk ? b : pk;
        }
        /* the hiss's envelope: up at once, down by 1/512 a block (~0.37 s); full hiss from ~-24 dB of the music */
        tp.env = pk > tp.env ? pk : tp.env - ((tp.env + 511) >> 9);
    }
    {
        /* wow + flutter: the delay at this block's end (Q8), reached linearly over the block */
        int32_t dw = t * t * 2, df = t * 3, drv = 4096 + t * 24, mk = (4096 << 12) / drv, kl = 28200 - t * 121;
        int32_t hz = (((t * t) >> 6) * (tp.env < 2048 ? tp.env : 2048)) >> 11, d1, dd, d;
        if ((tp.ph_wow >> 29) != ((tp.ph_wow + TP_WOW_INC) >> 29))   /* the wow's depth wanders: a new aim 4 times */
            tp.drift_to = (int32_t)(noise32(&tp.rng) >> 18) - 8192; /* a cycle, +-25 %, reached over ~1.5 s (a */
        tp.drift += (tp.drift_to - tp.drift) >> 11;                 /* faster step would be a pitch glitch) */
        tp.ph_wow += TP_WOW_INC;
        tp.ph_flut += TP_FLUT_INC;
        d1 = TP_C + mulq15(osc_sine(tp.ph_wow), (dw * (32768 + tp.drift)) >> 15)
                  + mulq15(osc_sine(tp.ph_flut), df);
        dd = n ? (d1 - tp.d_prev) / (int32_t)n : 0;     /* (n is a block, never 0: guarded all the same) */
        d = tp.d_prev;
        for (i = 0; i < n; i++) {
            uint32_t di, nz = noise32(&tp.rng);         /* (16 bits of hiss for each channel) */
            int32_t f;
            d += dd;
            di = (uint32_t)d >> 8;
            f = d & 255;
            for (k = 0; k < 2u; k++) {
                int32_t *b = tp_buf[k], s0, s1, y;
                b[tp.w & TP_MASK] = ch[k][i];
                s0 = b[(tp.w - di) & TP_MASK];
                s1 = b[(tp.w - di - 1u) & TP_MASK];
                y = s0 + (((s1 - s0) >> 1) * f >> 7);   /* (>> 1 first: |s| may reach 2^21) */
                /* the saturation. DIRGE: clamped at 2^18 (GHOULBOX: 2^21, whose half times the drive overflows 32 bits);
                 * the tanh is flat out from ~2^17 on, so the sound is the same */
                y = ((softclip(((clamp(y, -(1 << 18), 1 << 18) >> 1) * drv) >> 12) * mk) >> 12) << 1;
                tp.lp1[k] += mulq15(y - tp.lp1[k], kl);     /* the high cut, two poles */
                tp.lp2[k] += mulq15(tp.lp1[k] - tp.lp2[k], kl);
                tp.hiss_lp[k] += ((((int32_t)((nz >> (16u * k)) & 0xFFFFu) - 32768) * hz >> 17) - tp.hiss_lp[k]) >> 1;
                ch[k][i] = tp.lp2[k] + tp.hiss_lp[k];
            }
            tp.w++;
        }
        tp.d_prev = d1;
    }
}
