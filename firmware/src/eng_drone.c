/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE */
/* DRONE (DIRGE): four saws that never quite agree. Two at the note and two an octave down, pushed apart by DTN and
 * pulled about by DRFT (one slow random walk, each saw following it its own way), a sine SUB an octave under the
 * lower pair, BRTH (low-passed noise, the air in it), DRV (tanh before the filter) and dirge.h's low-pass, whose
 * rounded steps keep a long low drone free of DC. The saws are polyBLEP (dsp.c osc_saw).
 *   DTN   0 .. 25 cents between the saws of each pair
 *   SUB   the sine's level
 *   DRFT  the wander: 0 still .. ~12 cents, a new target every 0.7 .. 3 s, followed with a ~0.7 s lag
 *   LOW   the lower pair's level (0: the upper pair alone, thin; 127: as loud as it)
 *   CUT   the low-pass's cutoff (+ FLT), RES its resonance
 *   DRV   drive into the filter: 1x .. 4x
 *   BRTH  noise under the saws
 * Voice state: ph[0..2] saws 1..3, s[4] saw 4 (a phase kept in an int), s[5] the sub's phase; s[0], s[1] the filter;
 * s[2] the random state (seeded at the note); s[3] the walk's target (high 16 bits) and its countdown (low 16); s[6]
 * where the walk is (Q19); s[7] the breath's low-pass. */

static void drone_note_on(track_t *t, voice_t *v)
{
    uint32_t seed = (v->note + 1u) * 0x9E3779B9u ^ v->age * 0x85EBCA6Bu;
    (void)t;
    v->ph[1] = v->ph[0] + 0x55555555u;              /* the saws a third of a cycle apart: no click of four edges */
    v->ph[2] = v->ph[0] + 0x2AAAAAAAu;
    v->s[4] = (int32_t)(v->ph[0] + 0x80000000u);
    v->s[5] = 0;
    v->s[0] = v->s[1] = 0;
    v->s[2] = (int32_t)(seed ? seed : 1u);
    v->s[3] = 0;                                    /* a new target at once */
    v->s[6] = 0;
    v->s[7] = 0;
}

/* inc moved by c16 sixteenths of a cent (+-): 1 ct = 2.367 / 4096 (dsp.c cents_inc), so c16 x 0.1479 / 4096, as
 * 9694 / 2^16 (a 64-bit product, no division: smooth to a fraction of a cent) */
static inline uint32_t drone_det(uint32_t inc, int32_t c16)
{
    return inc + (uint32_t)(int32_t)(((int64_t)(int32_t)(inc >> 4) * (c16 * 9694)) >> 24);
}

static void drone_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    int32_t ic1 = v->s[0], ic2 = v->s[1], rng = v->s[2], walk = v->s[6], br = v->s[7], i;
    uint32_t ph0 = v->ph[0], ph1 = v->ph[1], ph2 = v->ph[2], ph3 = (uint32_t)v->s[4], phs = (uint32_t)v->s[5];
    int32_t det16 = p[P_E0] * 25 * 16 / 127 / 2;    /* half each way, 1/16 cent */
    int32_t sub = p[P_E1] * 258, low = p[P_E3] * 258, brth = p[P_E7] * 160;
    int32_t drv = p[P_E6], drive = 4096 + drv * 96; /* Q12: 1x .. ~4x */
    int32_t d16, cut = (p[P_E4] << 8) + m->cutoff;
    uint32_t hi = m->inc, lo = hi >> 1, inc0, inc1, inc2, inc3, incs = hi >> 2;
    tsvf_t flt;
    /* the walk, once a block (as eng_noise.c's drift): a new random target every 0.7 .. 3 s, followed with a lag */
    {
        int32_t tg = v->s[3] >> 16, cnt = v->s[3] & 0xFFFF;
        if (--cnt <= 0) {
            uint32_t r = noise32(&rng);
            tg = (int32_t)(r >> 16) - 32768;
            cnt = 1024 + (int32_t)(r & 3071u);
        }
        v->s[3] = (int32_t)((uint32_t)tg << 16 | (uint32_t)cnt);
        walk += ((tg << 4) - walk) >> 10;            /* Q19 */
    }
    d16 = ((walk >> 11) * p[P_E2] * 3) >> 9;        /* DRFT 127, the walk at its edge: ~+-12 cents (190 / 16) */
    /* each saw its own way along the walk: +1, -0.7, +0.4, -1 */
    inc0 = drone_det(hi, det16 + d16);
    inc1 = drone_det(hi, -det16 - ((d16 * 179) >> 8));
    inc2 = drone_det(lo, -det16 + ((d16 * 102) >> 8));
    inc3 = drone_det(lo, det16 - d16);
    tsvf_coef(&flt, cut, p[P_E5]);
    for (i = 0; i < (int32_t)n; i++) {
        int32_t up = osc_saw(ph0, inc0) + osc_saw(ph1, inc1);           /* +-65536 */
        int32_t dn = osc_saw(ph2, inc2) + osc_saw(ph3, inc3);
        int32_t s = (up >> 2) + mulq15(dn >> 2, low);                   /* +-32768 */
        ph0 += inc0;
        ph1 += inc1;
        ph2 += inc2;
        ph3 += inc3;
        s += mulq15(osc_sine(phs), sub) >> 1;
        phs += incs;
        if (brth) {                                 /* white through a one-pole (~2 kHz): breath, not hiss */
            br += ((int32_t)(noise32(&rng) >> 17) - 16384 - br) >> 3;
            s += mulq15(br, brth) >> 1;
        }
        if (drv)
            s = softclip((s * drive) >> 12);       /* (|s| < 2^16, drive < 2^14: fits) */
        s = soft_knee(tsvf_lp_r(&flt, s >> 1, &ic1, &ic2), 16000) << 1;
        out[i] += voice_amp(s, m, (uint32_t)i);
    }
    v->ph[0] = ph0;
    v->ph[1] = ph1;
    v->ph[2] = ph2;
    v->s[4] = (int32_t)ph3;
    v->s[5] = (int32_t)phs;
    v->s[0] = ic1;
    v->s[1] = ic2;
    v->s[2] = rng;
    v->s[6] = walk;
    v->s[7] = br;
}

static const preset_t DRONE_PRESETS[] = {
    {"DIRGE", {40, 50, 30, 127, 70, 24, 0, 0}, {90, 90, 127, 100}, 0, 0, FX(0, 20, 10, 60), PAT(5)},
    {"BLACK MASS", {20, 90, 15, 127, 40, 40, 10, 0}, {100, 90, 127, 110}, 0, 0, FX(0, 0, 0, 80), PAT(5)},
    {"SHIVER", {80, 20, 110, 90, 85, 30, 0, 20}, {80, 90, 127, 100}, 0, 0, FX(0, 40, 20, 60), PAT(5)},
    {"FURNACE", {55, 60, 40, 127, 60, 70, 110, 10}, {40, 90, 127, 80}, 10, 0, FX(30, 0, 20, 40), PAT(5)},
    {"ASH WIND", {30, 0, 60, 60, 55, 20, 0, 90}, {110, 90, 127, 115}, 0, 0, FX(0, 30, 10, 90), PAT(5)},
};

static const engine_t ENG_DRONE = {
    .name = "DRONE",
    .page_title = {"OSC", "TONE"},
    .edit = {
        {"DTN", F_INT, 0, 127, 40, 0, 0},
        {"SUB", F_PCT, 0, 127, 50, 0, 0},
        {"DRFT", F_PCT, 0, 127, 30, 0, 0},
        {"LOW", F_PCT, 0, 127, 127, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 70, 0, 0},
        {"RES", F_PCT, 0, 127, 24, 0, 0},
        {"DRV", F_PCT, 0, 127, 0, 0, 0},
        {"BRTH", F_PCT, 0, 127, 0, 0, 0},
    },
    .presets = DRONE_PRESETS,
    .npresets = NELEM(DRONE_PRESETS),
    .note_on = drone_note_on,
    .render = drone_render,
    .knob = {P_E0, P_E4, P_E1, P_E2},   /* DTN CUT SUB DRFT: step 1's DETUNE TONE SUB DRIFT */
    .poly = 4,                          /* four saws a voice: 4 voices (16 saws) at most, the CPU of 8 ANALOG ones */
    .keep = 0xF3,                       /* a retrigger keeps the filter, saw 4 and the sub, the walk, the breath */
};
