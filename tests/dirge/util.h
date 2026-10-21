/* SPDX-License-Identifier: GPL-3.0-only */
/* What the host tests share: a render's hash (FNV-1a 64 of its 16-bit samples, as Felucca's golden files), its
 * numbers, the health checks, and a WAV of it to listen to (build/tests/NAME.wav). */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char *name;
  const int16_t *s; /* interleaved when ch = 2 */
  uint32_t n;       /* samples (all channels) */
  int ch;
} render_t;

static int failures;

static uint64_t fnv1a64(const int16_t *s, uint32_t n) {
  uint64_t h = 0xcbf29ce484222325ull;
  for (uint32_t i = 0; i < n; i++) {
    uint16_t v = (uint16_t)s[i];
    h = (h ^ (v & 0xFFu)) * 0x100000001b3ull;
    h = (h ^ (v >> 8)) * 0x100000001b3ull;
  }
  return h;
}

typedef struct {
  int32_t peak;
  double rms, mean, diff_ratio; /* diff_ratio: rms of the first difference / rms, ~1.41 for white noise */
  uint32_t clipped;
} stats_t;

static stats_t stats(const int16_t *s, uint32_t n, int ch, uint32_t from, uint32_t to) { /* frames from..to */
  stats_t st = {0};
  double sq = 0, sum = 0, dsq = 0;
  uint32_t m = 0;
  for (uint32_t f = from; f < to && f * ch < n; f++) {
    for (int c = 0; c < ch; c++) {
      int32_t v = s[f * ch + c];
      int32_t a = v < 0 ? -v : v;
      if (a > st.peak) {
        st.peak = a;
      }
      if (a >= 32767) {
        st.clipped++;
      }
      sq += (double)v * v;
      sum += v;
      if (f > from) {
        double d = v - s[(f - 1) * ch + c];
        dsq += d * d;
      }
      m++;
    }
  }
  if (m) {
    st.rms = sqrt(sq / m);
    st.mean = sum / m;
    st.diff_ratio = st.rms > 0 ? sqrt(dsq / m) / st.rms : 0;
  }
  return st;
}

#define CHECK(r, cond, ...)                                    \
  do {                                                         \
    if (!(cond)) {                                             \
      fprintf(stderr, "FAIL %s: " #cond ": ", (r)->name);       \
      fprintf(stderr, __VA_ARGS__);                            \
      fprintf(stderr, "\n");                                   \
      failures++;                                              \
    }                                                          \
  } while (0)

static void w32(FILE *f, uint32_t v) { fwrite(&v, 4, 1, f); }
static void w16(FILE *f, uint16_t v) { fwrite(&v, 2, 1, f); }

static void write_wav(const render_t *r, const char *dir) {
  char path[512];
  snprintf(path, sizeof path, "%s/%s.wav", dir, r->name);
  for (char *p = path + strlen(dir) + 1; *p; p++) {
    if (*p == '/') {
      *p = '-';
    }
  }
  FILE *f = fopen(path, "wb");
  if (!f) {
    return;
  }
  uint32_t bytes = r->n * 2;
  fwrite("RIFF", 1, 4, f);
  w32(f, 36 + bytes);
  fwrite("WAVEfmt ", 1, 8, f);
  w32(f, 16);
  w16(f, 1);
  w16(f, (uint16_t)r->ch);
  w32(f, 44100);
  w32(f, 44100u * 2u * (uint32_t)r->ch);
  w16(f, (uint16_t)(2 * r->ch));
  w16(f, 16);
  fwrite("data", 1, 4, f);
  w32(f, bytes);
  fwrite(r->s, 2, r->n, f);
  fclose(f);
}

/* the golden line (stdout) and the WAV; the caller adds its own checks */
static void report(const render_t *r) {
  printf("%s %016llx\n", r->name, (unsigned long long)fnv1a64(r->s, r->n));
  const char *dir = getenv("WAV_DIR");
  if (dir && *dir) {
    write_wav(r, dir);
  }
}

static int16_t sat16(int32_t v) { return (int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v); }
