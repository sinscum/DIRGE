/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE */
/* BASS (DIRGE): the darksynth mono bass. One or two oscillators and a square an octave under them, DRV (tanh) into a
 * 24 dB resonant low-pass with its own snappy envelope: the pluck and growl of a synthwave / darksynth bassline, as a
 * MONO or LEGATO track plays it (the presets are MONO; POLY plays chords of it, 4 voices at most).
 *   WAVE  SAW, SQR, PULSE (25 %) or DUO (a saw and a square together)
 *   DTN   the second oscillator, 0 .. 30 cents above the first (0: one oscillator, the second silent)
 *   SUB   the square an octave down
 *   CUT   the low-pass's cutoff (+ FLT, the ENV DEST / LFO / matrix modulation), RES its resonance (second stage)
 *   ENV   how far the filter's own envelope opens the cutoff (up to ~80 notes), by the note's velocity
 *   DCY   that envelope's decay, 5 ms .. 10 s (the envelopes' times): short for a pluck, long for a swell
 *   DRV   drive into the filter: 1x .. 8x
 * The filter: two of dirge.h's rounded SVF stages in a row (24 dB), the first without resonance, its coefficients
 * once a control block from CUT + the modulation + the envelope. The oscillators are polyBLEP (dsp.c osc_saw,
 * osc_pulse). Voice state: ph[0] osc 1, ph[1] osc 2, ph[2] the sub; s[0] s[1] the first stage, s[2] s[3] the second;
 * s[4] the filter envelope (Q15, from the velocity at the note, falling). A retrigger keeps the filters (keep). */
enum { BW_SAW, BW_SQR, BW_PULSE, BW_DUO, BW_N };
static const char *const N_BASS_WAVE[BW_N] = {"SAW", "SQR", "PULSE", "DUO"};

static void bass_note_on(track_t *t, voice_t *v)
{
    (void)t;
    v->ph[1] = v->ph[0] + 0x40000000u;              /* the second a quarter cycle on: no doubled edge at the start */
    v->ph[2] = v->ph[0];
    v->s[4] = 32767 * (int32_t)(v->vel ? v->vel : 100u) / 127;   /* the filter envelope from the velocity */
}

/* one oscillator of WAVE w at ph (inc its step) */
static inline int32_t bass_osc(uint32_t w, uint32_t ph, uint32_t inc)
{
    switch (w) {
    case BW_SQR:
        return osc_pulse(ph, inc, 0x80000000u);
    case BW_PULSE: {
        int32_t q = osc_pulse(ph, inc, 0x40000000u);
        return q + (q >> 1);                        /* (as loud as the saw) */
    }
    case BW_DUO:
        return (osc_saw(ph, inc) + osc_pulse(ph, inc, 0x80000000u)) >> 1;
    default:
        return osc_saw(ph, inc);
    }
}

static void bass_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t w = (uint32_t)p[P_E0] < BW_N ? (uint32_t)p[P_E0] : BW_SAW, two = p[P_E1] > 0;
    uint32_t inc1 = m->inc, inc2 = drone_det(inc1, p[P_E1] * 30 * 16 / 127), incs = inc1 >> 1;
    uint32_t ph1 = v->ph[0], ph2 = v->ph[1], phs = v->ph[2], len, i;
    int32_t a1 = v->s[0], a2 = v->s[1], b1 = v->s[2], b2 = v->s[3], env = v->s[4];
    int32_t sub = p[P_E2] * 200, drv = p[P_E7], drive = 4096 + drv * 225;   /* Q12: 1x .. ~8x */
    int32_t cut = (p[P_E3] << 8) + m->cutoff + ((env * p[P_E5] * 160) >> 15);
    tsvf_t f1, f2;
    tsvf_coef(&f1, cut, 0);
    tsvf_coef(&f2, cut, p[P_E4]);
    for (i = 0; i < n; i++) {
        int32_t s = bass_osc(w, ph1, inc1);         /* +-32768 */
        if (two)
            s = (s + bass_osc(w, ph2, inc2)) >> 1;
        s += mulq15(osc_pulse(phs, incs, 0x80000000u), sub) >> 1;
        ph1 += inc1;
        ph2 += inc2;
        phs += incs;
        if (drv)
            s = softclip((s * drive) >> 12);       /* (|s| < 2^16, drive < 2^15: fits) */
        s = tsvf_lp_r(&f1, s >> 1, &a1, &a2);
        s = soft_knee(tsvf_lp_r(&f2, s, &b1, &b2), 16000) << 1;
        out[i] += voice_amp(s, m, i);
    }
    len = TIME_MS_X10[p[P_E6] & 127] * 441u;        /* DCY: 100 x its samples (TIME_MS_X10 x 4.41); the block takes n /
                                                     * samples of the way down, exponentially (env < 2^15, n x 100 =
                                                     * 3200: fits 32 bits; the divisor never under n x 100, never 0) */
    {
        int32_t d = (int32_t)((uint32_t)env * (n * 100u) / (len > n * 100u ? len : n * 100u));
        env -= d ? d : env > 0;                     /* (the last steps one at a time: it reaches 0) */
    }
    v->ph[0] = ph1;
    v->ph[1] = ph2;
    v->ph[2] = phs;
    v->s[0] = a1;
    v->s[1] = a2;
    v->s[2] = b1;
    v->s[3] = b2;
    v->s[4] = env > 0 ? env : 0;
}

static const preset_t BASS_PRESETS[] = {
    /*                WAVE      DTN SUB CUT RES ENV DCY DRV      ATK DEC SUS REL  fenv mono */
    {"PERTURB",     {BW_SAW,    20, 60, 30, 50, 90, 50, 40}, {0, 60, 100, 20}, 0, 1, FX(0, 0, 10, 15), PAT(8)},
    {"BRUT",        {BW_DUO,    35, 40, 25, 70, 110, 35, 90}, {0, 50, 90, 15}, 0, 1, FX(20, 0, 0, 10), PAT(8)},
    {"GRAVE",       {BW_SQR,    0, 90, 15, 20, 40, 70, 20}, {0, 80, 110, 30}, 0, 1, FX(0, 0, 0, 10), PAT(8)},
    {"SIREN",       {BW_PULSE,  10, 30, 40, 100, 70, 25, 50}, {0, 40, 80, 10}, 0, 1, FX(0, 0, 20, 10), PAT(8)},
    {"HEARSE",      {BW_SAW,    50, 70, 50, 30, 30, 90, 70}, {5, 90, 115, 40}, 0, 1, FX(10, 20, 0, 30), PAT(8)},
    {"BONESAW",     {BW_DUO,    90, 0, 60, 40, 60, 40, 127}, {0, 60, 100, 20}, 0, 1, FX(30, 0, 15, 20), PAT(8)},
};

static const engine_t ENG_BASS = {
    .name = "BASS",
    .page_title = {"OSC", "FILT"},
    .edit = {
        {"WAVE", F_ENUM, 0, BW_N - 1, BW_SAW, N_BASS_WAVE, 0},
        {"DTN", F_INT, 0, 127, 20, 0, 0},
        {"SUB", F_PCT, 0, 127, 60, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 30, 0, 0},
        {"RES", F_PCT, 0, 127, 50, 0, 0},
        {"ENV", F_PCT, 0, 127, 90, 0, 0},
        {"DCY", F_TIME, 0, 127, 50, 0, 0},
        {"DRV", F_PCT, 0, 127, 40, 0, 0},
    },
    .presets = BASS_PRESETS,
    .npresets = NELEM(BASS_PRESETS),
    .note_on = bass_note_on,
    .render = bass_render,
    .knob = {P_E3, P_E4, P_E5, P_E7},   /* CUT RES ENV DRV */
    .poly = 4,                          /* (two oscillators, a sub, two filter stages a voice) */
    .keep = 0x0F,                       /* a retrigger keeps the two filter stages */
};
