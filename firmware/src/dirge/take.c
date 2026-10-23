/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE (after Skiff's take.c, illuminus5/skiff, GPL-3.0, itself on Felucca's code) */
/* TAKE (DIRGE; Skiff's TAKES): while a track plays and is not recording, what is played on it (the keys and their
 * chords, MIDI, its arpeggiator: every note REC would record) and the knob moves on it are logged, each stamped with
 * the pass of the track's loop and the step REC would put it in (the step playing, or the next one when past the
 * middle of the playing one), with the nudge REC would give it (rec_nudge). REC held + C4 (ui_layer.c, take_commit in
 * dirge/take_commit.c) puts the last complete pass into the track: its notes as REC writes them, a held note tied on,
 * the knob moves as automation; one undo (SAVE held).
 * Included by seq.c after rec_note (take_note: from input_on / input_off and the arpeggiator, in the audio ISR where
 * the keys, MIDI and the arp play; take_knob: motion.c motion_capture, the main loop). Each entry is written whole
 * before its count is published. The log is in the pool (2.5 KB); nothing logged until PLAY, a new log from PLAY */
#define TAKE_EV 256u                                  /* notes on and off: ~128 notes, the oldest overwritten */
#define TAKE_KV 64u                                   /* knob moves (one per pass, step and parameter) */
typedef struct { uint16_t pass; uint8_t trk, step, note, vel; int8_t off; uint8_t rsv; } take_ev_t;   /* vel 0: the
                                                       * note's end; off: its nudge as REC would write it */
typedef struct { uint16_t pass; uint8_t trk, step, param, rsv; int16_t value; } take_kv_t;
static take_ev_t take_ev[TAKE_EV] __attribute__((section(".pool")));
static take_kv_t take_kv[TAKE_KV] __attribute__((section(".pool")));
static volatile uint32_t take_ev_n;                   /* entries ever written (the slot: n % TAKE_EV) */
static uint32_t take_kv_n;                            /* (main loop only) */
static volatile uint16_t take_pass[NTRK];             /* each track's pass since PLAY, from 1 (seq.c seq_enter: its
                                                       * step 0; PLAY parks it on its last step, so step 0 counts 1) */
static volatile uint16_t take_lost[NTRK];             /* 1 + the latest pass of a track's entry overwritten (0: none) */

/* PLAY (seq_start): a new log */
static void take_clear(void)
{
    uint32_t k;
    take_ev_n = 0;
    take_kv_n = 0;
    for (k = 0; k < NTRK; k++)
        take_pass[k] = take_lost[k] = 0;
}

/* where track t is now, as rec_note places a note: *pass, the step, *off its nudge */
static uint32_t take_stamp(const track_t *t, uint32_t *pass, int32_t *off)
{
    uint32_t len = t->p[P_SLEN] > 0 ? (uint32_t)t->p[P_SLEN] : 1u, idx = t->seq_idx % len;
    uint32_t period = div_samples((uint32_t)t->p[P_SDIV]), cur = step_samples(t, period, t->seq_idx);
    uint32_t next = t->seq_pos > cur / 2u;
    *pass = take_pass[trk_index(t)];
    *off = rec_nudge(t->seq_pos >= cur ? 0 : next ? -(int32_t)(cur - t->seq_pos) : (int32_t)t->seq_pos, period);
    if (next) {                                       /* the next step; past the pass's last: the next pass */
        if (idx + 1u >= len)
            ++*pass;
        idx = (idx + 1u) % len;
    }
    return idx;
}

/* a note on (vel > 0) or off played on track t, not recorded: logged (audio ISR) */
static __attribute__((noinline)) void take_note(track_t *t, uint32_t note, uint32_t vel)
{
    uint32_t pass, step, i;
    int32_t off;
    if (!note || !song.playing || rec_on(t))
        return;                                       /* (no note: an arp's none; stopped, or REC records it) */
    step = take_stamp(t, &pass, &off);
    i = take_ev_n % TAKE_EV;
    if (take_ev_n >= TAKE_EV && take_ev[i].pass + 1u > take_lost[take_ev[i].trk % NTRK])
        take_lost[take_ev[i].trk % NTRK] = (uint16_t)(take_ev[i].pass + 1u);   /* (that pass: a partial take) */
    take_ev[i].pass = (uint16_t)pass;
    take_ev[i].trk = (uint8_t)trk_index(t);
    take_ev[i].step = (uint8_t)step;
    take_ev[i].note = (uint8_t)note;
    take_ev[i].vel = (uint8_t)vel;
    take_ev[i].off = (int8_t)off;
    __asm__ volatile("" ::: "memory");                /* (the entry whole before its count is published) */
    take_ev_n++;
}

/* a knob move on track t's parameter id (motion.c motion_capture, not recording; main loop): the last value of each
 * pass, step and parameter */
static void take_knob(track_t *t, uint32_t id, int16_t value)
{
    uint32_t pass, step, k = trk_index(t), i, n = take_kv_n < TAKE_KV ? take_kv_n : TAKE_KV;
    int32_t off;
    if (!song.playing || rec_on(t))
        return;
    step = take_stamp(t, &pass, &off);                /* (a knob move: on the grid, its nudge unused) */
    for (i = 0; i < n; i++)
        if (take_kv[i].pass == pass && take_kv[i].trk == k && take_kv[i].step == step && take_kv[i].param == id) {
            take_kv[i].value = value;
            return;
        }
    i = take_kv_n++ % TAKE_KV;
    take_kv[i].pass = (uint16_t)pass;
    take_kv[i].trk = (uint8_t)k;
    take_kv[i].step = (uint8_t)step;
    take_kv[i].param = (uint8_t)id;
    take_kv[i].value = value;
}
