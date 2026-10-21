/* SPDX-License-Identifier: GPL-3.0-only */
/* A pretend FM-1 on the computer: the firmware (every .c in src/, unchanged) against fm1.h, with the platform's own sound
 * helpers and drawing (platform/common.c, as the emulator has them) and a scripted panel. Each script is a render:
 * one pass of loop() a millisecond, audio() every 256 frames (as platform/web.c), the output (stereo) hashed
 * against tests/golden.txt and checked: no clipping, no DC, silence after the fade.
 *   tests/run.sh builds it with every .c in src/ */
#include "util.h"

/* the platform's libc.c, renamed out of the computer's way (fmt_int is what common.c wants of it) */
#define memset fm1_memset
#define memcpy fm1_memcpy
#define memcmp fm1_memcmp
#include "fm1.h"
#include "libc.c"
#undef memset
#undef memcpy
#undef memcmp

typedef struct { /* as gfx.c */
  uint8_t h;
  uint8_t pad;
  uint8_t first, last;
  const uint8_t *adv;
  const uint8_t *bw;
  const uint16_t *off;
  const uint8_t *data;
} felucca_font_t;
#include "felucca_font.h"
#include "common.c"

/* ---- the panel, as the script sets it for one pass of loop() */
static uint32_t now_ms, key_down, key_edge, key_up, btn_down, btn_edge, btn_up;
static int32_t enc_turn[ENC_COUNT];

uint32_t millis(void) { return now_ms; }
uint32_t micros(void) { return now_ms * 1000u; }
uint32_t keys(void) { return key_down; }
int key_held(int k) { return (int)(key_down >> k & 1u); }
int key_pressed(int k) { return (int)(key_edge >> k & 1u); }
int key_released(int k) { return (int)(key_up >> k & 1u); }
uint32_t buttons(void) { return btn_down; }
int button_held(int b) { return (int)(btn_down >> b & 1u); }
int button_pressed(int b) { return (int)(btn_edge >> b & 1u); }
int button_released(int b) { return (int)(btn_up >> b & 1u); }
int32_t encoder(int e) {
  int32_t d = enc_turn[e];
  enc_turn[e] = 0;
  return d;
}
uint32_t master_knob(void) { return 1023; }
void led_key(int k, int on) { (void)k, (void)on; }
void led_button(int b, int on) { (void)b, (void)on; }
void led_backlight(int level) { (void)level; }
int midi_read(uint8_t *status, uint8_t *d1, uint8_t *d2) { return (void)status, (void)d1, (void)d2, 0; }
void midi_send(uint8_t status, uint8_t d1, uint8_t d2) { (void)status, (void)d1, (void)d2; }
#if FOMA_MIDI_CLOCK
void clock_follow(int from) { (void)from; }
void clock_send(int on) { (void)on; }
void clock_tempo(uint32_t bpm100) { (void)bpm100; }
void clock_start(void) {}
void clock_continue(void) {}
void clock_stop(void) {}
int clock_events(const clock_event_t **ev) { return (void)ev, 0; }
uint32_t clock_bpm(void) { return 12000; }
#endif
#if FOMA_USB_AUDIO
const int32_t *audio_in(void) { return 0; }
#endif
static uint8_t store[SETTINGS_MAX];
static uint32_t stored;
int settings_load(void *p, uint32_t n) {
  if (!stored || n > stored) {
    return -1;
  }
  memcpy(p, store, n);
  return 0;
}
int settings_save(const void *p, uint32_t n) {
  memcpy(store, p, n > SETTINGS_MAX ? SETTINGS_MAX : n);
  stored = n;
  return 0;
}
void print(const char *s) { (void)s; }

/* ---- the scripts: what happens at which millisecond */
enum { END, PRESS_KEY, PRESS_BTN, TURN }; /* (an empty entry is the end) */
typedef struct {
  uint32_t ms;
  int what, which, amount;
} ev_t;
typedef struct {
  const char *name;
  uint32_t ms;     /* how long */
  uint32_t on_ms;  /* the drone sounds from here ... */
  uint32_t off_ms; /* ... until here, then must fade to silence by the end */
  ev_t ev[12];
} script_t;

static const script_t SCRIPTS[] = {
    {"drone/default", 10000, 2500, 6000, {{0, PRESS_BTN, BTN_PLAY, 0}, {6000, PRESS_BTN, BTN_PLAY, 0}}},
    {"drone/everything-up",
     10000,
     2500,
     6000,
     {{0, TURN, ENC_KNOB1, 127},
      {0, TURN, ENC_KNOB2, 127},
      {0, TURN, ENC_KNOB3, 127},
      {0, TURN, ENC_KNOB4, 127},
      {1, PRESS_BTN, BTN_OCT_DOWN, 0},
      {2, PRESS_BTN, BTN_OCT_DOWN, 0},
      {3, PRESS_KEY, 0, 0},
      {6000, PRESS_KEY, 0, 0}}},
    {"drone/dark-and-still",
     10000,
     2500,
     6000,
     {{0, TURN, ENC_KNOB1, -127},
      {0, TURN, ENC_KNOB2, -127},
      {0, TURN, ENC_KNOB3, -127},
      {0, TURN, ENC_KNOB4, -127},
      {1, PRESS_KEY, 26, 0},
      {6000, PRESS_BTN, BTN_PLAY, 0}}},
};

static int16_t out16[2 * 10 * 44100 + 512];

static void run(const script_t *sc) {
  static int32_t blk[512];
  uint32_t frames = sc->ms * 441u / 10u, done = 0, ms_at = 0, e = 0;
  memset(&store, 0, sizeof store);
  stored = 0;
  now_ms = 0;
  setup();
  while (done < frames) {
    uint32_t block_end_ms = (done + 256u) * 10u / 441u;
    for (; ms_at < block_end_ms; ms_at++) { /* one pass of loop() a millisecond */
      now_ms = ms_at;
      key_edge = btn_edge = 0;
      for (; e < 12 && sc->ev[e].what != END && sc->ev[e].ms == ms_at; e++) {
        const ev_t *v = &sc->ev[e];
        if (v->what == PRESS_KEY) {
          key_edge |= 1u << v->which;
        } else if (v->what == PRESS_BTN) {
          btn_edge |= 1u << v->which;
        } else {
          enc_turn[v->which] += v->amount;
        }
      }
      loop();
    }
    memset(blk, 0, sizeof blk);
    audio(blk, 256);
    for (uint32_t i = 0; i < 512 && 2 * done + i < 2 * frames; i++) {
      out16[2 * done + i] = sat16(blk[i]);
    }
    done += 256;
  }

  render_t r = {sc->name, out16, 2 * frames, 2};
  report(&r);
  uint32_t on0 = sc->on_ms * 441u / 10u, off0 = sc->off_ms * 441u / 10u;
  stats_t on = stats(out16, r.n, 2, on0, off0), tail = stats(out16, r.n, 2, frames - 22050, frames);
  CHECK(&r, on.peak > 2000 && on.peak < 30000, "peak %d while it drones", on.peak);
  CHECK(&r, on.rms > 800, "rms %.0f while it drones", on.rms);
  CHECK(&r, on.clipped == 0, "%u samples clipped", on.clipped);
  CHECK(&r, fabs(on.mean) < 0.02 * on.peak + 20, "DC %.1f", on.mean);
  CHECK(&r, tail.peak == 0, "not silent at the end: peak %d in the last 0.5 s", tail.peak);
  fprintf(stderr, "  %-22s peak %5d (%.1f dBFS)  rms %5.0f  DC %6.1f\n", sc->name, on.peak, 20 * log10(on.peak / 32768.0), on.rms,
          on.mean);
}

int main(void) {
  for (unsigned i = 0; i < sizeof SCRIPTS / sizeof SCRIPTS[0]; i++) {
    run(&SCRIPTS[i]);
  }
  return failures ? 1 : 0;
}
