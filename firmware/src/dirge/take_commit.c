/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE (after Skiff's ui_take.c, illuminus5/skiff, GPL-3.0) */
/* TAKE (DIRGE; Skiff's TAKES): REC held + C4 (ui_layer.c) puts the last complete pass of the selected track's log
 * (dirge/take.c) into the track. An overdub: the notes go into the steps as REC writes them (beside what a step holds;
 * MONO / LEGATO / UNISON one note; a DRUM track its lanes' hits), a held note ties on into empty steps only, the knob
 * moves become automation; nothing already there is wiped. One undo (SAVE held: the steps and automation as before).
 * Included by ui.c after load_end (the undo copy). Main loop only. */

/* the hold of the note on in log entry i (pass want): the steps it ties into, its step + 1 .. the result - 1 (the
 * note's end, or the next note on the track: a new note ends the hold, as REC); held past the pass: to its end */
static uint32_t take_hold_end(uint32_t k, uint32_t i, uint32_t n, uint32_t want, uint32_t len)
{
    const take_ev_t *e = &take_ev[i % TAKE_EV];
    uint32_t j;
    for (j = i + 1u; j < n; j++) {
        const take_ev_t *f = &take_ev[j % TAKE_EV];
        if (f->trk != k)
            continue;
        if (f->pass != want)
            return len;                               /* held into the next pass: to the end (no ties wrap) */
        if (f->vel && f->step > e->step)
            return f->step;                           /* another note: the hold ends before it */
        if (!f->vel && f->note == e->note)
            return f->step > e->step ? f->step : e->step + 1u;   /* let go (stamped as REC: before a step's middle
                                                       * is that step, not held into it) */
    }
    return len;                                       /* still held */
}

/* note into step s of track t, as rec_note writes it (no timing: the step is known, off its nudge) */
static void take_put(const track_t *t, step_t *s, uint32_t note, uint32_t vel, int32_t off)
{
    uint32_t k;
    if (drum_track(t)) {                              /* DRUM: into the grid */
        uint32_t l = drum_lane(note);
        if (s->time != ST_NOTE || (!s->n && !s->hit)) {
            s->n = s->hit = s->acc = 0;
            s->flags = 0;
            s->vel = 0;
            step_set_nudge(s, off);
        }
        s->time = ST_NOTE;
        if (note == DRUM_LANE_NOTE[l]) {
            s->hit |= (uint8_t)(1u << l);
            if (vel > 110)
                s->acc |= (uint8_t)(1u << l);
        } else {
            for (k = 0; k < s->n && s->note[k] != note; k++)
                ;
            if (k == s->n && s->n < 4u)
                s->note[s->n++] = (uint8_t)note;
            if (vel > 110)
                s->flags |= SF_ACCENT;
        }
        if (vel > s->vel)
            s->vel = (uint8_t)vel;
        return;
    }
    if (s->time != ST_NOTE || !s->n || trk_vmode(t) != V_POLY) {
        s->n = 0;                                     /* a fresh step */
        s->flags = 0;
        s->vel = 0;
        step_set_nudge(s, off);
    }
    for (k = 0; k < s->n && s->note[k] != note; k++)
        ;
    if (k == s->n) {
        if (s->n < 4u)
            s->n++;
        s->note[s->n - 1u] = (uint8_t)note;
    }
    s->time = ST_NOTE;
    if (vel > 110)
        s->flags |= SF_ACCENT;
    if (vel > s->vel)
        s->vel = (uint8_t)vel;
}

/* REC held + C4 on track t would take (0) or why not: 1 not a whole pass played yet, 2 REC armed, 3 a song plays, 4
 * stopped with nothing logged (ui_layer.c: the map's TAKE row, its key's light) */
static uint32_t take_ready(const track_t *t)
{
    if (chain_busy())
        return 3u;
    if (rec_on(t))
        return 2u;
    if (take_pass[trk_index(t)] < 2u)
        return song.playing || take_ev_n ? 1u : 4u;
    return 0u;
}

/* REC held + C4 on track k */
static void take_commit(uint32_t k)
{
    track_t *t = &trk[k % NTRK];
    uint32_t n = take_ev_n, len = t->p[P_SLEN] > 0 ? (uint32_t)t->p[P_SLEN] : 1u, first, want, i, s, end;
    uint32_t notes = 0, moves = 0, full = 0;
    static step_t tmp[NSTEP] __attribute__((section(".pool")));   /* (off the main loop's stack and the RAM) */
    char b[24];
    if (len > NSTEP)
        len = NSTEP;
    if (chain_busy()) {
        ui_message("STOP TO EDIT");                   /* (a song plays its sections' patterns) */
        return;
    }
    if (rec_on(t)) {
        ui_message("DISARM REC FOR A TAKE");          /* (REC records it already) */
        return;
    }
    if (take_pass[k % NTRK] < 2u) {
        ui_message("PLAY ONE LOOP FIRST");            /* not a whole pass played since PLAY (the first is pass 1) */
        return;
    }
    want = take_pass[k % NTRK] - 1u;                  /* the pass before the one playing (or stopped in) */
    first = n > TAKE_EV ? n - TAKE_EV : 0u;
    memcpy(tmp, t->step, sizeof tmp);
    for (i = first; i < n; i++) {
        const take_ev_t *e = &take_ev[i % TAKE_EV];
        if (e->trk != k || e->pass != want || !e->vel || e->step >= len)
            continue;
        take_put(t, &tmp[e->step], e->note, e->vel, e->off);
        notes++;
        if (drum_track(t))
            continue;                                 /* (hits: no holds) */
        end = take_hold_end(k, i, n, want, len);
        for (s = e->step + 1u; s < end && s < len; s++)
            if (!tmp[s].n && !tmp[s].hit) {           /* a held note ties on into empty steps only */
                tmp[s].time = ST_TIE;
                tmp[s].flags = 0;
                tmp[s].vel = 0;
            }
    }
    for (i = 0; i < (take_kv_n < TAKE_KV ? take_kv_n : TAKE_KV); i++)
        moves += take_kv[i].trk == k && take_kv[i].pass == want && take_kv[i].step < len;
    if (!notes && !moves) {
        ui_message("NOTHING PLAYED");
        return;
    }
    undo.keep = 0;                                    /* its own undo copy: SAVE held puts back steps and automation */
    load_begin(t, UNDO_PAT);                          /* (which drops the track's automation: put back, as a take */
    if (motion_replace_track(t, &undo.motion_backup)) /* adds to what is there) */
        full = 1;
    fm1_irq_off();                                    /* the audio ISR must not see half a pattern */
    memcpy(t->step, tmp, sizeof tmp);
    t->seq_active = 1;
    fm1_irq_on();
    for (i = 0; i < (take_kv_n < TAKE_KV ? take_kv_n : TAKE_KV); i++) {
        const take_kv_t *m = &take_kv[i];
        if (m->trk == k && m->pass == want && m->step < len && motion_set_event(t, m->step, m->param, m->value))
            full = 1;
    }
    load_end(t);
    str_cpy(b, "TAKE T1 ", sizeof b);                 /* "TAKE T2 14 NOTES"; PARTIAL: the log ran over; AUTO FULL: */
    b[6] = (char)('1' + k % NTRK);                    /* some knob moves found no room */
    if (take_lost[k % NTRK] > want || full) {
        ui_say(b, take_lost[k % NTRK] > want ? "PARTIAL" : "AUTO FULL");
    } else {
        fmt_int(b + 8, (int32_t)notes);
        ui_say(b, notes == 1u ? " NOTE" : " NOTES");
    }
    str_cpy(ui.msg2, "HOLD [SAVE]: UNDO", sizeof ui.msg2);
}
