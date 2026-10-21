/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE */
/* DIRGE's DSP, beside Felucca-Engines' dsp.c (firmware/engines/src/dsp.c, MIT: osc_saw osc_pulse osc_tri osc_sine,
 * tsvf_coef tsvf_lp, softclip soft_knee, noise32, cents_inc ...), included by engines.c right after it. All static
 * inline, fixed point. tests/dirge/ renders and checks each block on the computer.
 *   tsvf_lp_r   tsvf_lp with its shifts rounded: no DC creeping into a long low drone at a low cutoff
 *   noise.h     white, pink, brown, crackle and shift-register noise */
#ifndef DIRGE_H
#define DIRGE_H

/* the SVF low-pass as tsvf_lp, its shifts rounded (as Felucca's NOISE engine does): the floors of tsvf_lp walk
 * the integrators off zero when the cutoff is low, a DC offset under a long drone */
static inline int32_t tsvf_lp_r(const tsvf_t *c, int32_t in, int32_t *ic1, int32_t *ic2)
{
    int32_t v3 = in - *ic2;
    int32_t v1 = (c->a1 * *ic1 + c->a2 * v3 + 4096) >> 13;
    int32_t v2 = *ic2 + ((c->a2 * *ic1 + c->a3 * v3 + 4096) >> 13);
    *ic1 = clamp(2 * v1 - *ic1, -150000, 150000);
    *ic2 = clamp(2 * v2 - *ic2, -150000, 150000);
    return v2;
}

#include "noise.h"
#endif
