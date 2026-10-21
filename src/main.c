/* DIRGE: a fixed-point detuned drone, on DIRGE's DSP library (src/dsp/dsp.h).
 *
 *   keys        set the drone's root and start it; the same key again fades it out
 *   PLAY        starts / stops the drone on the last root
 *   OCT- OCT+   the octave (kept in flash, with the knobs)
 *   KNOB1       DETUNE  0..25 cents between the saws
 *   KNOB2       TONE    a resonant low-pass, ~220 Hz to open
 *   KNOB3       SUB     a sine an octave under the root
 *   KNOB4       DRIFT   slow random wander of each saw's pitch
 *
 * Four saws: two at the root and two an octave down, pushed apart by DETUNE and pulled
 * around by DRIFT, the pairs panned left and right. A sine sub under them, a lowpass over
 * them, a slow fade in and out. The saws are band-limited (polyBLEP): no aliasing whine on
 * high notes.
 *
 * What you can call: platform/fm1.h. The rules (no float, keep audio() short): CLAUDE.md. */
#include "fm1.h"

#include "dsp/dsp.h"

#define SAWS 4
#define CENTS_Q24 9691 /* one cent as a ratio above 1, Q24: ln(2) / 1200 * 2^24 */
#define FADE_IN 95     /* per frame, Q23: full level in ~2 s */
#define FADE_OUT 63    /* per frame, Q23: silent in ~3 s */

/* ---- written by loop(), read by audio() */
static volatile uint32_t saw_inc[SAWS]; /* phase steps, detuned and drifted */
static volatile uint32_t sub_inc;
static volatile int32_t tone_cut; /* the low-pass's cutoff, 0..127 << 8 (dsp.h tsvf_coef) */
static volatile int32_t sub_lvl; /* Q15 */
static volatile int32_t gate;    /* target level, Q23: 0 = fading out */

/* ---- audio() only */
static uint32_t saw_ph[SAWS], sub_ph;
static int32_t lp[4];        /* the low-passes' state: left, right */
static volatile int32_t amp; /* the level now, Q23; written by audio(), read by loop() */

static struct {
  int8_t octave;
  uint8_t knob[4];
} saved = {0, {40, 70, 50, 30}};

static int root = KEY_FIRST_NOTE; /* the drone's MIDI note */

#define RESONANCE 24 /* 0..127: a little, so TONE's sweeps speak */

/* ---- the sound: runs in the audio interrupt. Keep it short: no screen, no flash */
void audio(int32_t *out, uint32_t frames) {
  int32_t a = amp, g = gate, sl = sub_lvl;
  uint32_t inc[SAWS], si = sub_inc;
  tsvf_t flt;
  if (!a && !g) {
    return;
  }
  tsvf_coef(&flt, tone_cut, RESONANCE); /* once a block */
  for (int s = 0; s < SAWS; s++) {
    inc[s] = saw_inc[s];
  }
  for (uint32_t i = 0; i < frames; i++) {
    if (a < g) {
      a = a + FADE_IN > g ? g : a + FADE_IN;
    } else if (a > g) {
      a = a - FADE_OUT < g ? g : a - FADE_OUT;
    }
    int32_t saw[SAWS];
    for (int s = 0; s < SAWS; s++) {
      saw[s] = osc_saw(saw_ph[s], inc[s]) >> 1; /* +-16384 */
      saw_ph[s] += inc[s];
    }
    sub_ph += si;
    int32_t sub = (sine_i(sub_ph) * sl) >> 17; /* +-8192 at full SUB */
    /* saws 0 and 2 lean left, 1 and 3 right: +-49152, then a quarter: headroom for what comes after */
    int32_t l = saw[0] + saw[2] + ((saw[1] + saw[3]) >> 1);
    int32_t r = saw[1] + saw[3] + ((saw[0] + saw[2]) >> 1);
    l = tsvf_lp_r(&flt, (l >> 2) + sub, &lp[0], &lp[1]);
    r = tsvf_lp_r(&flt, (r >> 2) + sub, &lp[2], &lp[3]);
    int32_t lvl = a >> 8; /* Q15 */
    out[2 * i] += (l * lvl) >> 15;
    out[2 * i + 1] += (r * lvl) >> 15;
  }
  amp = a;
}

/* ---- the drone's parameters, from the knobs: loop() only */
static int32_t drift[SAWS]; /* each saw's wander, +-256 = all of DRIFT */

static uint32_t detuned(uint32_t inc, int32_t cents256) {
  /* inc * 2^(cents/1200), to first order: plenty for +-50 cents */
  int64_t ratio = (1 << 24) + (((int64_t)cents256 * CENTS_Q24) >> 8);
  return (uint32_t)(((uint64_t)inc * (uint64_t)ratio) >> 24);
}

static void update_sound(void) {
  static const int8_t SPREAD[SAWS] = {1, -1, -1, 1};   /* which way DETUNE pushes each saw */
  int32_t detune = saved.knob[0] * 25 * 256 / 127 / 2; /* half each way, cents x 256 */
  int32_t wander = saved.knob[3] * 12 * 256 / 127;     /* up to 12 cents of drift */
  static uint32_t walked;
  int walk = millis() - walked >= 40; /* a step every 40 ms, whatever loop()'s pace */
  if (walk) {
    walked = millis();
  }
  for (int s = 0; s < SAWS; s++) {
    if (walk) { /* a slow random walk, pulled back to the middle */
      drift[s] += (int32_t)(random32() % 129) - 64 - drift[s] / 64;
    }
    int32_t d = drift[s] > 256 ? 256 : drift[s] < -256 ? -256
                                                       : drift[s];
    uint32_t base = note_inc(s < 2 ? root : root - 12);
    saw_inc[s] = detuned(base, SPREAD[s] * detune + d * wander / 256);
  }
  sub_inc = note_inc(root - 12);
  tone_cut = (40 << 8) + saved.knob[1] * (87 << 8) / 127; /* ~220 Hz .. 16 kHz */
  sub_lvl = saved.knob[2] * 32767 / 127;
}

/* ---- the screen */
#define ACCENT RGB(170, 40, 40)
#define DIM RGB(90, 90, 90)

static void draw_centered(int y, const char *s, uint16_t color, int font) {
  rect(0, y, SCREEN_W, text_height(font), BLACK);
  text((SCREEN_W - text_width(s, font)) / 2, y, s, color, font);
}

static void draw_note(int on) {
  static const char *const NAME[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  char s[24], *p = s;
  for (const char *n = NAME[root % 12]; *n;) {
    *p++ = *n++;
  }
  format_int(p, root / 12 - 1);
  draw_centered(80, s, on ? WHITE : DIM, FONT_BIG);
  char o[24] = "OCTAVE ";
  format_int(o + 7, saved.octave);
  draw_centered(134, o, DIM, FONT_SMALL);
}

static void draw_knob(int k) { /* four columns of 60 px: the label, the value big */
  static const char *const NAME[4] = {"DETUNE", "TONE", "SUB", "DRIFT"};
  char v[12];
  int x = k * 60;
  format_int(v, saved.knob[k]);
  rect(x, 180, 60, 52, BLACK);
  text(x + (60 - text_width(NAME[k], FONT_SMALL)) / 2, 180, NAME[k], DIM, FONT_SMALL);
  text(x + (60 - text_width(v, FONT_BIG)) / 2, 198, v, ACCENT, FONT_BIG);
}

/* detents turned this pass -> steps: a slow turn is 1 a detent, a fast spin up to 4 */
static int accel(int e, int32_t detents) {
  static uint32_t last[ENC_COUNT];
  uint32_t now = millis(), dt = now - last[e];
  if (!detents) {
    return 0;
  }
  last[e] = now;
  return detents * (dt < 25 ? 4 : dt < 60 ? 2
                                          : 1);
}

static void drone(int on) {
  gate = on ? 32767 << 8 : 0;
  led_button(BTN_PLAY, on);
  draw_note(on);
}

void setup(void) {
  settings_load(&saved, sizeof saved);
  if (saved.octave < -2 || saved.octave > 2) {
    saved.octave = 0;
  }
  for (int k = 0; k < 4; k++) {
    saved.knob[k] = saved.knob[k] > 127 ? 127 : saved.knob[k];
  }
  root = KEY_FIRST_NOTE + 12 * saved.octave;
  clear(BLACK);
  draw_centered(20, "DIRGE", ACCENT, FONT_BIG);
  draw_centered(56, "detuned drone", DIM, FONT_SMALL);
  for (int k = 0; k < 4; k++) {
    draw_knob(k);
  }
  update_sound();
  drone(0);
}

void loop(void) {
  static uint32_t changed_at;
  static int dirty;
  int base = KEY_FIRST_NOTE + 12 * saved.octave;
  uint8_t st, d1, d2;

  for (int k = 0; k < KEY_COUNT; k++) {
    if (key_pressed(k)) {
      int same = gate && root == base + k;
      root = base + k;
      drone(!same);
    }
  }
  while (midi_read(&st, &d1, &d2)) { /* a note in sets the root and starts it; notes off leave it droning */
    if ((st & 0xF0) == 0x90 && d2) {
      root = d1;
      drone(1);
    }
  }
  if (button_pressed(BTN_PLAY)) {
    drone(!gate);
  }

  int oct = saved.octave + button_pressed(BTN_OCT_UP) - button_pressed(BTN_OCT_DOWN);
  if (oct != saved.octave && oct >= -2 && oct <= 2) {
    saved.octave = (int8_t)oct;
    draw_note(gate != 0);
    dirty = 1;
    changed_at = millis();
  }
  for (int k = 0; k < 4; k++) {
    int v = saved.knob[k] + accel(ENC_KNOB1 + k, encoder(ENC_KNOB1 + k));
    v = v < 0 ? 0 : v > 127 ? 127
                            : v;
    if (v != saved.knob[k]) {
      saved.knob[k] = (uint8_t)v;
      draw_knob(k);
      dirty = 1;
      changed_at = millis();
    }
  }
  led_button(BTN_OCT_DOWN, saved.octave < 0);
  led_button(BTN_OCT_UP, saved.octave > 0);
  update_sound();

  if (dirty && !gate && !amp && millis() - changed_at > 2000) { /* saving stalls the sound: wait for silence */
    settings_save(&saved, sizeof saved);
    dirty = 0;
  }
}
