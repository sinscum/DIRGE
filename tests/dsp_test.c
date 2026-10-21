/* SPDX-License-Identifier: GPL-3.0-only */
/* DIRGE's DSP library on the computer: each building block rendered (2 s, mono), hashed against tests/golden.txt
 * and checked: level, DC, colour, aliasing. tests/run.sh builds and runs it.
 *   WAV_DIR=build/tests build/tests/dsp_test     (the renders as WAVs too) */
#include "util.h"

#include "../src/dsp/dsp.h"

#define N (2 * FS)
static int16_t buf[N];

static uint32_t hz_inc(uint32_t hz) { return (uint32_t)(((uint64_t)hz << 32) / FS); }

/* an exponential sweep's pitch at frame i: from p0 to p1, in 1/16 semitones */
static uint32_t sweep_inc(uint32_t i, int32_t p0, int32_t p1) { return pitch_inc((uint32_t)(p0 + (int64_t)(p1 - p0) * i / N)); }

static render_t done(const char *name) {
  render_t r = {name, buf, N, 1};
  report(&r);
  return r;
}

static void healthy(const render_t *r, int32_t peak_min, int32_t peak_max) {
  stats_t s = stats(r->s, r->n, 1, 0, N);
  CHECK(r, s.peak >= peak_min && s.peak <= peak_max, "peak %d, wanted %d..%d", s.peak, peak_min, peak_max);
  CHECK(r, fabs(s.mean) < 0.01 * s.peak + 50, "DC %.1f (peak %d)", s.mean, s.peak);
  CHECK(r, s.clipped == 0, "%u samples at full scale", s.clipped);
}

/* energy below ~1.5 kHz (two SVF low-passes): for a 5 kHz tone, all of it is aliasing */
static double low_energy(const int16_t *s, uint32_t n) {
  tsvf_t c;
  int32_t a1 = 0, a2 = 0, b1 = 0, b2 = 0;
  double e = 0;
  tsvf_coef(&c, 86 << 8, 0); /* ~1.5 kHz */
  for (uint32_t i = 0; i < n; i++) {
    int32_t y = tsvf_lp(&c, tsvf_lp(&c, s[i], &a1, &a2), &b1, &b2);
    if (i > 4410) {
      e += (double)y * y;
    }
  }
  return e;
}

static void oscillators(void) {
  uint32_t ph = 0;
  for (uint32_t i = 0; i < N; i++) { /* 20 Hz .. ~10 kHz */
    uint32_t inc = sweep_inc(i, 15 * 16, 135 * 16 > 2047 ? 2047 : 135 * 16);
    buf[i] = sat16(osc_saw(ph, inc) >> 1);
    ph += inc;
  }
  healthy(&(render_t){"osc/saw-sweep", buf, N, 1}, 15000, 20000);
  done("osc/saw-sweep");

  ph = 0;
  for (uint32_t i = 0; i < N; i++) { /* 110 Hz, the width from 50 % to 5 % and back */
    uint32_t inc = hz_inc(110), w = (uint32_t)(i < N / 2 ? i : N - i) * 2u;
    uint32_t pw = 0x80000000u - (uint32_t)(((uint64_t)w * 0x73333333u) / N);
    buf[i] = sat16(osc_pulse(ph, inc, pw) >> 1);
    ph += inc;
  }
  healthy(&(render_t){"osc/pulse-pwm", buf, N, 1}, 6000, 17000);
  done("osc/pulse-pwm");

  ph = 0;
  for (uint32_t i = 0; i < N; i++) {
    buf[i] = sat16(osc_tri(ph) >> 1);
    ph += hz_inc(220);
  }
  healthy(&(render_t){"osc/tri", buf, N, 1}, 16000, 16384);
  done("osc/tri");

  ph = 0;
  for (uint32_t i = 0; i < N; i++) {
    buf[i] = sat16(sine_i(ph) >> 1);
    ph += hz_inc(440);
  }
  healthy(&(render_t){"osc/sine", buf, N, 1}, 16000, 16384);
  done("osc/sine");

  /* polyBLEP against the naive saw at 5 kHz: the band-limited one must alias far less */
  static int16_t naive[N];
  uint32_t inc = hz_inc(4987);
  ph = 0;
  for (uint32_t i = 0; i < N; i++) {
    buf[i] = sat16(osc_saw(ph, inc) >> 1);
    naive[i] = sat16(((int32_t)(ph >> 16) - 32768) >> 1);
    ph += inc;
  }
  double eb = low_energy(buf, N), en = low_energy(naive, N);
  render_t r = {"osc/saw-aliasing", buf, N, 1};
  CHECK(&r, eb < en * 0.35, "aliasing below 1.5 kHz: polyBLEP %.3g, naive %.3g (ratio %.2f)", eb, en, eb / en);
  fprintf(stderr, "  polyBLEP saw at 5 kHz: %.1f dB less aliasing than the naive one\n", 10 * log10(en / eb));
}

static void filters(void) {
  uint32_t ph = 0;
  int32_t ic1 = 0, ic2 = 0;
  tsvf_t c;
  for (uint32_t i = 0; i < N; i++) { /* a 55 Hz saw, the cutoff swept up and down, resonance high */
    if (i % CTL == 0) {
      uint32_t t = i < N / 2 ? i : N - i;
      tsvf_coef(&c, (int32_t)((uint64_t)t * (2 * 120 << 8) / N), 110);
    }
    buf[i] = sat16(tsvf_lp_r(&c, osc_saw(ph, hz_inc(55)) >> 2, &ic1, &ic2));
    ph += hz_inc(55);
  }
  healthy(&(render_t){"filter/svf-sweep", buf, N, 1}, 6000, 32000);
  done("filter/svf-sweep");

  /* the rounded low-pass at its lowest cutoff, fed a quiet sine: no DC creeps in */
  ic1 = ic2 = 0;
  ph = 0;
  tsvf_coef(&c, 0, 0);
  double sum = 0;
  for (uint32_t i = 0; i < N; i++) {
    buf[i] = sat16(tsvf_lp_r(&c, sine_i(ph) >> 6, &ic1, &ic2));
    sum += buf[i];
    ph += hz_inc(30);
  }
  render_t r = done("filter/svf-low-dc");
  CHECK(&r, fabs(sum / N) < 2, "DC %.2f at the lowest cutoff", sum / N);

  for (uint32_t i = 0; i < N; i++) { /* a sine driven from x 0.1 to x 8 into softclip */
    int32_t g = 410 + (int32_t)((uint64_t)i * (32768 - 410) / N);
    buf[i] = sat16(softclip((int32_t)(((int64_t)sine_i(ph) * g) >> 12)) >> 1);
    ph += hz_inc(110);
  }
  healthy(&(render_t){"shape/softclip-drive", buf, N, 1}, 15000, 16384);
  done("shape/softclip-drive");
}

static void noises(void) {
  noise_t nz;
  struct {
    const char *name;
    int kind;
    double dr_min, dr_max; /* the first difference's rms over the rms: white ~1.41, darker less */
  } K[] = {
      {"noise/white", 0, 1.35, 1.48},
      {"noise/pink", 1, 0.45, 1.1},
      {"noise/brown", 2, 0.0, 0.25},
  };
  for (unsigned k = 0; k < 3; k++) {
    noise_init(&nz, 0x2545F491u);
    for (uint32_t i = 0; i < N; i++) {
      buf[i] = sat16(K[k].kind == 0 ? noise_white(&nz) : K[k].kind == 1 ? noise_pink(&nz) : noise_brown(&nz));
    }
    render_t r = done(K[k].name);
    stats_t s = stats(buf, N, 1, 0, N);
    CHECK(&r, s.diff_ratio >= K[k].dr_min && s.diff_ratio <= K[k].dr_max, "colour: difference / rms %.2f, wanted %.2f..%.2f",
          s.diff_ratio, K[k].dr_min, K[k].dr_max);
    CHECK(&r, s.rms > 4000 && s.rms < 14000, "rms %.0f", s.rms);
    /* (pink and brown live mostly below ~40 Hz: over 2 s their mean wanders by chance, ~rms / 15; a bias would be more) */
    CHECK(&r, fabs(s.mean) < 0.1 * s.rms, "DC %.1f (rms %.0f)", s.mean, s.rms);
    CHECK(&r, s.peak < 28000, "peak %d", s.peak);
    fprintf(stderr, "  %-12s rms %5.0f  peak %5d  difference/rms %.2f\n", K[k].name, s.rms, s.peak, s.diff_ratio);
  }

  noise_init(&nz, 7);
  uint32_t hits = 0;
  for (uint32_t i = 0; i < N; i++) {
    nz.thr = noise_rate(i < N / 2 ? 20 : 600); /* sparse crackle, then a fire */
    buf[i] = sat16(noise_crackle(&nz));
    hits += buf[i] != 0;
  }
  render_t r = done("noise/crackle");
  CHECK(&r, hits > (10 + 600) * 1 / 2 && hits < (20 + 600) * 2, "%u impulses, wanted ~620", hits);

  lfsr_t l;
  for (uint32_t len = 0; len < 8; len += 7) { /* 23 bits at C6: hiss; 7 bits at C4: a metallic buzz */
    lfsr_init(&l, len, 0x1234567u);
    for (uint32_t i = 0; i < N; i++) {
      buf[i] = sat16(lfsr_tick(&l, pitch_inc(len ? 60 * 16 : 84 * 16) << 4));
    }
    render_t q = done(len ? "noise/lfsr-7" : "noise/lfsr-23");
    stats_t s = stats(buf, N, 1, 0, N);
    CHECK(&q, s.peak == 12000 && fabs(s.mean) < 1500, "peak %d, DC %.0f", s.peak, s.mean);
  }
}

int main(void) {
  oscillators();
  filters();
  noises();
  return failures ? 1 : 0;
}
