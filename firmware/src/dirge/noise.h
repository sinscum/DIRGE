/* SPDX-License-Identifier: GPL-3.0-only */
/* DIRGE's noise sources, one sample a call, Q15 out: an rms of ~7000..9500, peaks under ~27000.
 * The arithmetic follows Felucca's NOISE engine (firmware/engines/src/eng_noise.c), taken out of its voice:
 *   noise_white    xorshift32, +-16384 (a uniform rms of ~9460)
 *   noise_pink     white through three leaky poles (Paul Kellet's "economy" filter): -3 dB an octave
 *   noise_brown    white through a leaky integrator: -6 dB an octave, a rumble that still has no DC
 *   noise_crackle  random impulses at a rate (noise_rate): dust, vinyl, a fire. Random size and sign
 *   lfsr_*         a Galois shift register clocked at a phase step (a pitch): 7..23 bits, from a metallic buzz to
 *                  NES / Atari-style noise. Two levels, no filter
 * Included by dirge.h (after Felucca-Engines dsp.c: it needs noise32, mulq15, clamp, soft_knee). */

typedef struct {
  int32_t rng;            /* xorshift state, never 0 */
  int32_t b0, b1, b2, br; /* the pink poles, the brown integrator */
  uint32_t thr;           /* crackle: impulses a sample, Q16 (noise_rate) */
} noise_t;

static inline void noise_init(noise_t *n, uint32_t seed) {
  n->rng = (int32_t)(seed ? seed : 1u);
  n->b0 = n->b1 = n->b2 = n->br = 0;
  n->thr = 0;
}

static inline int32_t noise_white(noise_t *n) { return (int32_t)(noise32(&n->rng) >> 17) - 16384; }

/* x * k / 2^15, rounded to nearest: the poles below feed back, and floors there add up to a DC offset */
static inline int32_t noise_mul(int32_t x, int32_t k) { return (x * k + 16384) >> 15; }

static inline int32_t noise_pink(noise_t *n) {
  int32_t w = noise_white(n);
  n->b0 = noise_mul(n->b0, 32690) + noise_mul(w, 3245);
  n->b1 = noise_mul(n->b1, 31555) + noise_mul(w, 9716);
  n->b2 = noise_mul(n->b2, 18678) + noise_mul(w, 34494);
  return (n->b0 + n->b1 + n->b2 + noise_mul(w, 6055)) >> 2;
}

static inline int32_t noise_brown(noise_t *n) {
  int32_t w = noise_white(n);
  n->br = noise_mul(n->br, 32580) + noise_mul(w, 3480);
  return soft_knee((n->br * 3) >> 2, 16000); /* x 3/4 and its rare peaks rounded off */
}

/* impulses a second (1 .. 40000) -> noise_t.thr */
static inline uint32_t noise_rate(uint32_t per_second) {
  if (per_second > 40000u) {
    per_second = 40000u;
  }
  return (per_second * 97391u) >> 16; /* rate x 65536 / 44100 */
}

static inline int32_t noise_crackle(noise_t *n) {
  uint32_t r = noise32(&n->rng);
  if ((r & 0xFFFFu) >= n->thr) {
    return 0;
  }
  int32_t a = 8192 + (int32_t)((r >> 18) & 0x3FFFu); /* 8192 .. 24575 */
  return (r & 0x20000u) ? a : -a;
}

/* ---- the shift register */
typedef struct {
  uint32_t reg, taps, ph;
} lfsr_t;

/* Galois taps of maximal-length registers (2^n - 1 clocks), as eng_noise.c's: 23 20 17 15 13 11 9 7 bits */
static const uint32_t LFSR_TAPS[8] = {0x420000u, 0x90000u, 1u << 16 | 1u << 13, 0x6000u, 0x100Du, 0x500u, 0x110u, 0x60u};

static inline void lfsr_init(lfsr_t *l, uint32_t len, uint32_t seed) { /* len 0..7: 23 bits .. 7 bits */
  uint32_t mask;
  l->taps = LFSR_TAPS[len & 7u];
  mask = l->taps; /* the register's own bits: the top tap down */
  mask |= mask >> 1;
  mask |= mask >> 2;
  mask |= mask >> 4;
  mask |= mask >> 8;
  mask |= mask >> 16;
  l->reg = (seed & mask) ? seed & mask : 1u;
  l->ph = 0;
}

/* one sample: the register steps each time the phase wraps (inc: a phase step, e.g. note_inc or pitch_inc) */
static inline int32_t lfsr_tick(lfsr_t *l, uint32_t inc) {
  uint32_t o = l->ph;
  l->ph += inc;
  if (l->ph < o) {
    l->reg = (l->reg >> 1) ^ (-(l->reg & 1u) & l->taps);
  }
  return (l->reg & 1u) ? 12000 : -12000;
}
