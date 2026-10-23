/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 DIRGE (ported from Skiff's tests/duck_test.c, illuminus5/skiff, GPL-3.0) */
/* Host test of DIRGE's DUCK (firmware/src/dirge/duck.c, FX > DUCK; Skiff's ducking) through the whole mix (hostsim.c,
 * mix_block). T1 holds a saw note, no sends; T4, a DRUM kit at LEVEL 0 (silent: only its hits count), plays a kick
 * on every beat and a closed hat on every 1/16, from PLAY (120 BPM).
 * 1. T1's gain (duck.c, each block's end): 100 % DEPTH all the way down 5 blocks into each kick (3 ms); back in a
 *    straight line over RELEASE (half way at half of it, whole at its end), 1 again before the next; the hats on
 *    the 1/16s between never duck it (a DRUM source: its KICK lane only).
 * 2. the output: T1's level (2 ms RMS, ducking on over the same render without it) as that gain says.
 * 3. DEPTH 50 %: half way down. No click: inside a block the gain moves in a straight line from the last block's.
 * 4. BY OFF, BY the track itself, a muted source: the output bit for bit as without ducking.
 * 5. a synth source (T2, notes on steps 1 and 9 only): each of its notes ducks T1, nothing between.
 * 6. the last track (T4) under the one before it; the parameters (kept by sound loads, automation, the page).
 * Each comparison renders the other side in a fork of the same state (as perform_test.c). Built and run by
 * tests/run_tests.sh. */
#define main hostsim_main
#include "hostsim.c"
#undef main

static int check(const char *what, int ok)
{
    printf("duck: %-76s %s\n", what, ok ? "ok" : "FAIL");
    return ok ? 0 : 1;
}

#define NB (2u * 44100u / CTL)                     /* blocks rendered: 2 s, a bar at 120 BPM */
#define REL 64                                     /* RELEASE 104 ms (TIME_MS_X10[64]: 4573 samples) */
static int32_t out_on[NB * CTL], out_off[NB * CTL];
static int32_t gain[NB];                           /* T1's gain at each block's end, Q15 */
static uint8_t step_at[NB];                        /* the step T4 (or T2) plays at each block's end */

/* T1 a saw note held, ducked by `src` at DEPTH amt; T4 kicks on the beats, hats on the 1/16s, silent; T2 (silent)
 * notes on steps 1 and 9 */
static void setup(uint32_t src, uint32_t amt)
{
    uint32_t i;
    memset(trk, 0, sizeof trk);
    memset(&motion, 0, sizeof motion);
    memset(duck, 0, sizeof duck);
    duck_hits = duck_on = 0;
    host_tracks_init();
    song.g[G_BPM] = 120;
    host_preset(&trk[0], 0, 0);                    /* ANALOG SAW LEAD */
    trk[0].p[P_CHOR] = trk[0].p[P_DLY] = trk[0].p[P_REV] = 0;
    trk[0].p[P_DKSRC] = (int16_t)src;
    trk[0].p[P_DKAMT] = (int16_t)amt;
    trk[0].p[P_DKREL] = REL;
    host_preset(&trk[1], 0, 0);
    trk[1].p[P_LEVEL] = 0;
    trk[1].step[0] = (step_t){{60}, 1, ST_NOTE, 0, 100};
    trk[1].step[8] = (step_t){{64}, 1, ST_NOTE, 0, 100};
    host_drums(&trk[3]);
    trk[3].p[P_LEVEL] = 0;
    for (i = 0; i < 16u; i++) {
        trk[3].step[i].time = ST_NOTE;
        trk[3].step[i].hit = (uint8_t)(1u << 3 | (i % 4u == 0u ? 1u : 0u));   /* HATCL, KICK on the beats */
    }
}
/* from PLAY, NB blocks into o[] (the left side); T1's gain and the step of track k each block */
static void render(int32_t *o, uint32_t k)
{
    uint32_t b, i;
    int32_t buf[2 * CTL];
    transport_req = 1;
    trk_note_on(&trk[0], 45, 100);
    for (b = 0; b < NB; b++) {
        mix_block(buf, CTL);
        for (i = 0; i < CTL; i++)
            o[b * CTL + i] = buf[2 * i];
        gain[b] = 32768 - duck[0].r1;
        step_at[b] = (uint8_t)trk[k].seq_idx;
    }
    transport_req = 2;
    mix_block(buf, CTL);
}
/* the same state rendered without ducking (BY OFF), in a fork, into out_off */
static int render_off(uint32_t k)
{
    int fd[2];
    pid_t pid;
    size_t got = 0;
    if (pipe(fd) || (pid = fork()) < 0)
        return 0;
    if (!pid) {
        trk[0].p[P_DKSRC] = 0;
        render(out_off, k);
        close(fd[0]);
        _exit(write(fd[1], out_off, sizeof out_off) == (ssize_t)sizeof out_off ? 0 : 1);
    }
    close(fd[1]);
    {
        FILE *f = fdopen(fd[0], "rb");
        got = fread(out_off, 1, sizeof out_off, f);
        fclose(f);
        waitpid(pid, 0, 0);
    }
    return got == sizeof out_off;
}
static double rms(const int32_t *x, uint32_t a)    /* 2 ms from sample a */
{
    double s = 0;
    uint32_t i;
    for (i = a; i < a + 88u && i < NB * CTL; i++)
        s += (double)x[i] * x[i];
    return sqrt(s / 88.0);
}
static double level(uint32_t a) { return rms(out_on, a) / (rms(out_off, a) + 1e-9); }
/* the blocks a hit started in: where track k's step became one of `steps` (bit i: step i; PLAY's first block too) */
static uint32_t hits_of(uint32_t steps, uint32_t *at)
{
    uint32_t b, n = 0;
    for (b = 0; b < NB; b++)
        if ((!b || step_at[b] != step_at[b - 1u]) && ((steps >> step_at[b]) & 1u))
            at[n++] = b;
    return n;
}

static int kicks(void)
{
    int bad = 0, ok, okh = 1, oko;
    uint32_t at[16], n, j, b, rel = (32768u + 228u) / 229u;   /* blocks to come back (229: 32768 x 32 / 4573) */
    setup(4, 127);
    ok = render_off(3);
    render(out_on, 3);
    n = hits_of(0x1111u, at);
    ok &= n == 4u && !at[0];                       /* (PLAY's first block: step 1, a kick) */
    for (j = 0; j < n; j++) {
        b = at[j];
        ok &= (!b || gain[b - 1u] == 32768) && gain[b] < 32768 && gain[b + 3u] > 0 && gain[b + 4u] < 40;
        ok &= abs(gain[b + 4u + rel / 2u] - 16384) < 400 && gain[b + 4u + rel] == 32768;
        for (b = at[j] + 5u + rel; b < (j + 1u < n ? at[j + 1u] : NB); b++)
            okh &= gain[b] == 32768;                /* (the hats' 1/16s) */
    }
    bad += check("T1 under T4's kicks: all the way down in 3 ms, back in a line over RELEASE, 1 by the next", ok);
    bad += check("  the hats between (T4 a DRUM kit: its KICK lane only) never duck", okh);
    oko = 1;
    for (j = 1; j < n; j++) {
        uint32_t s = at[j] * CTL;
        oko &= level(s - 200u) > 0.98 && level(s + 300u) < 0.08;   /* (down: the master's DC blocker rings on a few %) */
        oko &= fabs(level(s + 160u + rel * CTL / 2u) - 0.5) < 0.06 && fabs(level(s + 200u + rel * CTL) - 1.0) < 0.02;
    }
    bad += check("  the output: T1's level (2 ms RMS) down and back as the gain", oko);
    return bad;
}

static int depth_and_ramp(void)
{
    int bad = 0, ok;
    uint32_t b, i, at[16];
    int32_t lo = 32768, buf[CTL];
    setup(4, 64);
    render(out_on, 3);
    hits_of(0x1111u, at);
    for (b = 0; b < NB; b++)
        lo = gain[b] < lo ? gain[b] : lo;
    bad += check("DEPTH 50 %: half way down", abs(lo - 16384) < 300);
    duck[0].r0 = 0;                                /* a block from 1 to 0.75: a straight line */
    duck[0].r1 = 8192;
    for (i = 0; i < CTL; i++)
        buf[i] = 1 << 20;
    duck_apply(0, buf, CTL);
    ok = buf[0] == 1 << 20;
    for (i = 1; i < CTL; i++)
        ok &= buf[i] < buf[i - 1u] && abs((buf[i - 1u] - buf[i]) - (8192 / CTL) * 32) < 2;
    bad += check("  no click: inside a block the gain moves in a line from the last block's", ok && buf[CTL - 1u] > (3 << 18));
    (void)at;
    return bad;
}

static int none(void)
{
    static const uint8_t SRC[3] = {0, 1, 4};
    static const char *const WHAT[3] = {"BY OFF", "BY the track itself", "a muted source"};
    int bad = 0, i;
    uint32_t t, diff;
    char s[80];
    for (i = 0; i < 3; i++) {
        setup(SRC[i], 127);
        if (i == 2)
            trk[3].p[P_MUTE] = 1;
        diff = !render_off(3);
        render(out_on, 3);
        for (t = 0; t < NB * CTL; t++)
            diff += out_on[t] != out_off[t];
        snprintf(s, sizeof s, "%s (DEPTH 100 %%): bit for bit as without ducking", WHAT[i]);
        bad += check(s, !diff);
    }
    return bad;
}

static int synth(void)
{
    uint32_t at[16], n, b, j, ok;
    setup(2, 127);
    render(out_on, 1);
    n = hits_of(1u << 0 | 1u << 8, at);
    ok = n == 2u;
    for (b = 1, j = 0; b < NB; b++) {
        int down = j < n && b >= at[j] && b < at[j] + 160u;
        if (j < n && b == at[j] + 160u)
            j++;
        ok &= down ? (b == at[j] ? gain[b] < 32768 : 1) : gain[b] == 32768;
    }
    return check("a synth source (T2, notes on steps 1 and 9): each note ducks T1, nothing between", ok);
}


/* the last track (T4): ducked under the one before it (BY reaches every track) */
static int last_track(void)
{
    int bad = 0;
    uint32_t k = NTRK - 1u, s = NTRK - 2u, b, i;
    int32_t buf[2 * CTL], lo = 32768;
    setup(0, 0);
    host_preset(&trk[k], 0, 0);                    /* the last track a saw, the one before it the kicks */
    trk[k].p[P_CHOR] = trk[k].p[P_DLY] = trk[k].p[P_REV] = 0;
    trk[k].p[P_DKSRC] = (int16_t)(s + 1u);
    trk[k].p[P_DKAMT] = 127;
    trk[k].p[P_DKREL] = REL;
    host_drums(&trk[s]);
    trk[s].p[P_LEVEL] = 0;
    for (i = 0; i < 16u; i++) {
        trk[s].step[i].time = ST_NOTE;
        trk[s].step[i].hit = (uint8_t)(i % 4u == 0u ? 1u : 0u);   /* KICK on the beats */
    }
    transport_req = 1;
    trk_note_on(&trk[k], 45, 100);
    for (b = 0; b < NB; b++) {
        mix_block(buf, CTL);
        if (32768 - duck[k].r1 < lo)
            lo = 32768 - duck[k].r1;
    }
    transport_req = 2;
    mix_block(buf, CTL);
    bad += check("the last track ducked all the way under the one before it (BY reaches every track)",
                 lo < 40 && TP[P_DKSRC].max == (int16_t)NTRK);
    return bad;
}

/* DEPTH and REL can be automated, BY not (routing); FX > DUCK follows INSERT 2 with BY DEPTH REL (that a sound load
 * keeps them, ui.c param_kept: tests/ui_test.c) */
static int loads(void)
{
    uint32_t i;
    int ok = !motion_param(P_DKSRC) && motion_param(P_DKAMT) && motion_param(P_DKREL);
    ok &= P_DKSRC == P_ED_FX && P_ESYNC + 1 == P_DKAMT && P_DKREL + 1 == P_E0 && P_COUNT == 113;
    for (i = 0; i + 1u < NPAGES && !str_eq(PAGES[i].title, "INSERT 2"); i++)
        ;
    ok &= i + 1u < NPAGES && str_eq(PAGES[i + 1u].title, "DUCK") && PAGES[i + 1u].fam == FAM_FX &&
          PAGES[i + 1u].id[0] == P_DKSRC && PAGES[i + 1u].id[1] == P_DKAMT && PAGES[i + 1u].id[2] == P_DKREL;
    ok &= TP[P_DKSRC].max == (int16_t)NPART && TP[P_DKSRC].def == 0;
    return check("DEPTH / REL automatable (BY not), FX > DUCK after INSERT 2, BY in P_ED_FX's id, P_COUNT 113", ok);
}

int main(void)
{
    int bad = kicks() + depth_and_ramp() + none() + synth() + last_track() + loads();
    printf("%s\n", bad ? "DUCK TEST FAILED" : "ducking tests passed");
    return bad != 0;
}
