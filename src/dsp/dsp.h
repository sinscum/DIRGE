/* SPDX-License-Identifier: GPL-3.0-only */
/* DIRGE's DSP foundation: everything fixed point, every function static inline. Include this from the one .c file
 * that makes the sound (each file that includes it gets its own copy of the tables, in flash).
 *
 * From Felucca-Engines (lib/felucca-engines, MIT, unchanged):
 *   mulq15 mulq16 clamp                     fixed-point arithmetic
 *   osc_saw osc_pulse                       band-limited (polyBLEP) saw and pulse: (phase, increment[, width])
 *   osc_tri osc_sine sine_i                 triangle, table sine
 *   tsvf_coef tsvf_lp                       Simper's trapezoidal state-variable low-pass: stable at any setting
 *   softclip soft_knee                      tanh, and linear up to a knee then tanh
 *   noise32                                 xorshift32
 *   pitch_inc cents_inc                     phase steps from 1/16 semitones (+ cents)
 *   SINE SVF_G TANH_Q15 FS CTL              the tables (src/dsp/fe_tables.h, from its gen_engine_tables.py)
 * DIRGE's own, alongside:
 *   noise.h                                 white, pink, brown, crackle and shift-register noise
 *   tsvf_lp_r                               tsvf_lp with its shifts rounded: no DC creeping in at low cutoffs */
#ifndef DIRGE_DSP_H
#define DIRGE_DSP_H
#include <stdint.h>

/* felucca_engine.h wants the host's voice counts: DIRGE has no engine host yet, so one of each */
#ifndef NVOICE
#define NVOICE 1
#endif
#ifndef NPART
#define NPART 1
#endif
#include "fe_tables.h"
#include "../../lib/felucca-engines/src/felucca_engine.h"
#include "../../lib/felucca-engines/src/dsp.c"

/* the SVF low-pass as tsvf_lp, its shifts rounded (as Felucca's NOISE engine does): the floors of tsvf_lp walk
 * the integrators off zero when the cutoff is low, a DC offset under a long drone */
static inline int32_t tsvf_lp_r(const tsvf_t *c, int32_t in, int32_t *ic1, int32_t *ic2) {
  int32_t v3 = in - *ic2;
  int32_t v1 = (c->a1 * *ic1 + c->a2 * v3 + 4096) >> 13;
  int32_t v2 = *ic2 + ((c->a2 * *ic1 + c->a3 * v3 + 4096) >> 13);
  *ic1 = clamp(2 * v1 - *ic1, -150000, 150000);
  *ic2 = clamp(2 * v2 - *ic2, -150000, 150000);
  return v2;
}

#include "noise.h"
#endif
