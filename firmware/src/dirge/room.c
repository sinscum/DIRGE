/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE (after GHOULBOX's ambience.c, jasonpersinger/ghoulbox-fm1-dungeon-synth, GPL-3.0) */
/* ROOM (DIRGE, G_ROOM on FX > TAPE): the room tone of a dead channel under the mix, with no track and no voice, as
 * GHOULBOX's ambience is a cave's: into the dry mix and a share into the reverb's send, before the buses (fx.c
 * mix_block); it plays whether the song plays or not. ROOM 0 and faded out: nothing runs, the mix bit for bit as
 * without it. Its level glides (at most 256 a block, ~65 ms for the whole range, each block ending on its aim exactly):
 * turning it never clicks, and it fades out to 0.
 *   static   white noise, its own each side, through a one-pole low-pass (~5 kHz) less a slower one (~150 Hz): the hiss
 *            of a set tuned to nothing; its level on a slow random walk (seconds, ~-4 dB .. 0)
 *   hum      the mains: 50 Hz with its 2nd and 3rd (100, 150 Hz), the same both sides, well under the static
 *   crackle  GHOULBOX's: pops at random times (ROOM^2: up to ~20 a second at 127), a 0.3 .. 1.3 ms burst of the noise's
 *            first difference (bright), panned at random, one in eight a louder snap
 * ROOM is a level squared: the first half of the knob quiet. 127: ~18 dB under a full track. */
#define RM_HUM 4869970u                                 /* 50 Hz: 50 / FS x 2^32 */
static struct {
    int32_t rng, lv, lpl, lpr, hpl, hpr, walk, walk_to, env, dec, gl, gr, prev;
    uint32_t ph, wait;
    uint8_t on;
} rm;

static __attribute__((noinline)) void room_block(int32_t *l, int32_t *r, int32_t *rev, uint32_t n)
{
    int32_t a = song.g[G_ROOM], to = a * a * 23170 / (127 * 127), lv, dl, aim;
    uint32_t i;
    if (!a && !rm.lv) {                                 /* off, and faded out */
        rm.on = 0;
        return;
    }
    if (!rm.on) {                                       /* from silence */
        memset(&rm, 0, sizeof rm);
        rm.rng = 0x1F123BB5;
        rm.walk = rm.walk_to = 26000;
        rm.on = 1;
    }
    /* once a block: the static's level walk (toward a new aim when it gets there, ~0.7 s) */
    if (rm.walk - rm.walk_to < 256 && rm.walk_to - rm.walk < 256)
        rm.walk_to = 20000 + (int32_t)(noise32(&rm.rng) >> 19);   /* ~-4 dB .. 0 */
    rm.walk += (rm.walk_to - rm.walk) >> 10;
    lv = rm.lv;
    aim = lv + clamp(to - lv, -256, 256);               /* this block's aim on the way to the knob's level */
    dl = n ? (aim - lv) / (int32_t)n : 0;               /* (n is a block, never 0) */
    for (i = 0; i < n; i++) {
        int32_t nl = (int32_t)(noise32(&rm.rng) >> 17) - 16384, nr = (int32_t)(noise32(&rm.rng) >> 17) - 16384;
        int32_t sl, sr, hum, cl = 0, cr = 0, g;
        lv += dl;
        g = mulq15(lv, rm.walk);
        rm.lpl += mulq15(nl - rm.lpl, 18000);           /* the static: ~5 kHz low-pass less ~150 Hz's */
        rm.lpr += mulq15(nr - rm.lpr, 18000);
        rm.hpl += mulq15(rm.lpl - rm.hpl, 700);
        rm.hpr += mulq15(rm.lpr - rm.hpr, 700);
        sl = mulq15(rm.lpl - rm.hpl, g);
        sr = mulq15(rm.lpr - rm.hpr, g);
        rm.ph += RM_HUM;                                /* the hum: 50 Hz, its 2nd and 3rd */
        hum = (osc_sine(rm.ph) + (osc_sine(rm.ph * 2u) >> 1) + (osc_sine(rm.ph * 3u) >> 2)) >> 3;
        hum = mulq15(hum, lv);
        if (rm.wait)
            rm.wait--;
        else if (a) {                                   /* a pop, and the time to the next (uniform, mean FS / rate) */
            uint32_t u = noise32(&rm.rng), rate = (uint32_t)(a * a) * 20u * 256u / (127u * 127u) + 1u;   /* per s, x256 */
            int32_t pan = (int32_t)(noise32(&rm.rng) >> 17);
            rm.env = 6000 + (int32_t)(u & 0x3FFFu);
            rm.dec = 32767 - 32767 / (13 + (int32_t)((u >> 14) % 45u));   /* ~0.3 .. 1.3 ms */
            rm.gl = mulq15(32767 - pan, to) >> ((u >> 29) ? 1 : 0);   /* one in eight a snap (louder) */
            rm.gr = mulq15(pan, to) >> ((u >> 29) ? 1 : 0);
            rm.prev = (int32_t)(noise32(&rm.rng) >> 17) - 16384;
            rm.wait = noise32(&rm.rng) % (2u * FS * 256u / rate) + 1u;   /* (rate >= 1: no division by 0) */
        }
        if (rm.env) {                                   /* the pop: the noise's first difference (bright), fading */
            int32_t x = (int32_t)(noise32(&rm.rng) >> 17) - 16384, c = mulq15(x - rm.prev, rm.env);
            rm.prev = x;
            rm.env = rm.env > 64 ? mulq15(rm.env, rm.dec) : 0;
            cl = mulq15(c, rm.gl);
            cr = mulq15(c, rm.gr);
        }
        l[i] += sl + hum + cl;
        r[i] += sr + hum + cr;
        rev[i] += (sl + sr + cl + cr) >> 3;             /* a share into the reverb: the same room as the tracks */
    }
    rm.lv = aim;                                        /* (exactly: no remainder of the division left over) */
    if (!a && rm.lv <= 0) {                             /* faded out: silence from the next block */
        rm.lv = 0;
        rm.on = 0;
    }
}
