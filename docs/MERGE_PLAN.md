# Merge plan: DIRGE + Skiff + GR!MM

_Drafted 2026-10-23 from a hands-on review, then checked against the code (DIRGE `main` at 8987f10, Skiff `main` at
c01fa1e = Skiff 1.4)._

**DIRGE is the base.** Skiff is a code donor; GR!MM is a design reference only (Dinghy template: re-implement its ideas,
do not copy code). Everything stays GPL-3.0-only; credit Skiff and GR!MM in `CREDITS.md` / `LICENSING.md` for anything
ported or adapted.

| Repo | Role | Built on | License |
| --- | --- | --- | --- |
| sinscum/DIRGE | base, keeps its history | Felucca 1.6.1 | GPL-3.0-only |
| illuminus5/skiff | code donor, changes marked `(Skiff)` | Felucca 1.1.5, plus "the best of Felucca 1.2 to 1.6.1" pulled in by Skiff 1.3 | GPL-3.0-only |
| GRiMMBIENT/grimm | design reference | Felucca-Engines + Dinghy | GPL-3.0 |

## What the code check changed

The first draft assumed DIRGE had only Felucca's basics. Felucca 1.2 to 1.6.1 already carries much of what Skiff
added, so several "port" rows are already done:

- **SLICER** is in DIRGE (`slicer.c`, the same 203 lines as Skiff's: GATE / STUT, 16 patterns, P_SLCR..P_SLDEPTH).
- **PERFORM** is in DIRGE (`perform.c`: REPEAT, REVERSE, TAPE STOP, FREEZE, OCT UP / DN, FLANGER, PHASER, REV THROW,
  ECHO THROW on the FX layer).
- **Arp** is identical: the same 15 modes (OFF + 14), ARP 2 with SWING, CHANCE (P_APROB), HOLD, ORDER.
- **LFO** pages match: DIRGE has LFO, LFO 2, LFO DEST and MOD (the 4-slot matrix), SYNC / TRIG / POL.
- **AUTOMATION** (SEQ > AUTOMATION, motion locks) and **MIXER** pages are in DIRGE.
- **Undo**: DIRGE has a one-step undo (hold SAVE); Skiff keeps 8 levels across the tracks (`ui.c`). That one is a
  real port.
- **Distortion** is mostly there: FX's DIST is one soft clip, but the per-track **INSERT** (FX > INSERT, Felucca 1.5)
  already has SOFT, HARD, FOLD, FUZZ and CRUSH (bits and rate), plus PHASR, FLANG and CHOR, with a MIX. Skiff's DRIVE
  kinds would add HARD / FOLD / BITS to DIST itself, which matters only if a track needs an INSERT and a second
  distortion at once.
- **One-key chords** are there: SCL > CHORD (DIA3, DIA7, MAJ .. POW; 5 voicings) makes every key play a chord. What
  GR!MM adds is the gesture (hold SEL), not the feature.
- **Latch** is there for arpeggios: ARP 2 > HOLD.
- **Themes**: DIRGE already has BLOOD / CRYPT / NEON palettes.

What is genuinely Skiff-only:

| Feature | Skiff source | Size | Notes |
| --- | --- | --- | --- |
| DRIVE: HARD / FOLD / BITS + TONE | `fx.c` (P_DTYPE, P_DTONE) | ~70 lines | 2 new per-track parameters before P_E0 (projects map by count) |
| DUCK (sidechain) | `duck.c` + hooks in `fx.c`, `voice.c` | 65 + ~20 lines | 3 parameters (P_DKSRC, P_DKAMT, P_DKREL) |
| ENV 2, CHANCE, PLAY, ROLL pages | `params.c`, `ui_pgraph.c` (884), `ui_blocks.c` (415) | large | mostly UI; per-page graphs |
| TAKE (capture the last pass) | `take.c`, `ui_take.c` | 290 lines | runs in the audio ISR (note log) |
| Pattern pool, per-track lists A..P, scenes | `bank.c` (1390), `launch.c`, `editor_song.c` | large | own project format (SKF1 / SKF2) and flash map |
| Six tracks | `core.h` NTRK 6 and everything that counts tracks | very large | see the blockers |
| DRUM-X engine | `eng_drumx.c`, `drumx_voice.c` | 515 lines | Skiff numbers it 14, which is DIRGE's DRONE |
| HOME landing page | `ui_home.c` | 179 lines | |
| Preset tags (LIST by type) | `core.h` PT_*, `ui.c` | small | one stored byte per preset |
| 8-level undo | `ui.c` | medium | DIRGE keeps one level; Skiff keeps the ring in a `.ramx` section its `app.ld` adds |

## Blockers for the big ports

1. **Engine 14.** Skiff's DRUM-X and DIRGE's DRONE are both engine 14, and engine numbers are stored. A DRUM-X port
   must be appended after DRONE (15), and Skiff sounds that use DRUM-X would need their engine byte remapped.
2. **Flash map.** Skiff's pattern pool takes 0xD6000..0xEA000 (20 sectors). In DIRGE that range holds the main store
   (to 0xE0000), the OTA staging (0xE0000..0xE5000), the autosave (0xE5000..0xE7000) and the FM6 voice bank (0xE7000).
   The pool cannot be copied as is; it needs a new home or a smaller size, decided before any code moves.
3. **RAM.** Six tracks pushed Skiff's .data+.bss to 74.7 KB, and Skiff moved the RAM / POOL split to 96 KB / 348 KB
   (`app.ld`). DIRGE sits at 61 of 72 KiB RAM with ~40 KB of pool, which VAULT, TAPE, ROOM and WEAR rely on. Six
   tracks in DIRGE means the same split move, and then re-checking every DIRGE buffer in the pool.
4. **Formats.** NTRK is counted by the project format, the editor protocol and the web editor. DIRGE writes FUN10,
   Skiff writes SKF1 / SKF2. Six tracks or the pattern pool means a new DIRGE format (with FUN10 import), not a merge
   of Skiff's.
5. **Voice budget.** DIRGE today: 4 tracks sharing 8 voices (`core.h` NVOICE, `voice.c`). Skiff 1.3 raises the budget
   to 12 while the second core runs (VOICE_BUDGET); DIRGE does not have that yet. (CLAUDE.md's "12 voices" is Skiff's
   number, not DIRGE's.)

No shared git history: Skiff's 19 commits have no merge base with DIRGE, so ports are by hand (diff the files),
not cherry-picks.

## Revised port order

Small, self-contained wins first; the structural changes after a design note each. (Updated after the docs pass
found INSERT's distortions and the chord keys already in DIRGE.)

1. **DUCK** (`duck.c`): off by default, bit for bit when off. **Done** (`firmware/src/dirge/duck.c`, FX > DUCK after
   INSERT 2; BY reuses the unused id 8, DEPTH / REL are new: P_COUNT 113). FUN10 is now full for track parameters, so
   anything per-track after this (DRIVE's two, ENV 2) needs a format step: plan it with the six-track format (5).
2. **Preset tags / LIST** and the **ENV 2** page, if wanted after trying Skiff's.
3. **8-level undo and TAKE** (`ui.c`, `take.c`): CPU-check the ISR note log against `tests/target_budget.txt`. **Done**:
   an 8-copy undo ring in the pool (SAVE held as before, then OCT- / OCT+ for 3 s); TAKE on REC held + C4
   (`firmware/src/dirge/take.c`, `take_commit.c`; Skiff's SHIFT is HOME held, DIRGE's MENU).
4. **Voice budget 12 on two cores** (Skiff 1.3's VOICE_BUDGET), before any track-count change. **Done** (`core.h`
   VOICE_BUDGET; untested on hardware: CPU-check DRONE SUPER + VAULT + TAPE with 12 voices on the unit).
5. **Design note, then build**: six tracks + the RAM / POOL split + a new project format (design note below: does
   not fit without cuts; waiting on a decision). CPU-test DRONE + VAULT +
   TAPE with six tracks running.
6. **Design note, then build**: the pattern pool, per-track lists and scenes, with a DIRGE flash layout (design note
   below: needs 80 KB of flash, one user sample slot; waiting on a decision).
7. GR!MM-inspired UI: HOME with direct access to parameters (Skiff's HOME as the landing page), GRAIN as a send with
   FREEZE, a quicker gesture for the existing chord keys, a per-track note latch (beyond ARP HOLD).
8. DRUM-X as engine 15 (optional).
9. Plague-doctor theme.
10. Skiff's DRIVE kinds on DIST, only if INSERT's distortions are not enough in practice.

Each step: `tests/run_tests.sh` green (goldens unchanged unless the sound was meant to change), and a J_DIRGE render
where it makes sound.

## Design note: steps 5 and 6 (six tracks, the pattern pool)

Measured on `main` after step 4 (`./build.sh`, the target build):

| | Used | Free |
| --- | --- | --- |
| RAM (.data + .bss) | 61.5 of 72 KiB | ~12.2 KB |
| Pool (the same SRAM) | 346.8 of 360 KiB | ~21.8 KB |
| App image | 523.6 of 568 KB | ~44 KB |
| Flash store | projects 0x97000 (4 x A/B), user samples 0xA0000 (3 x 80 KB), user presets 0xDC000, OTA 0xE0000, autosave 0xE5000, FM6 bank 0xE7000 | no free 80 KB run |

**Six tracks: what two more tracks cost.**

| Item | Per track | Two more | Where |
| --- | --- | --- | --- |
| `track_t` (voices, steps, state) | 1840 B | 3.7 KB | RAM |
| The song chain's four slots (`song_chain.c` `chain.source`: steps of every track) | 3 KB | 6.1 KB | RAM |
| Engine state region (`engines.c` `eng_mem`, sized by PHYS) | 12.9 KB | 25.8 KB | pool |
| SLICER STUT buffer (`slicer.c` `sl_buf`; PERFORM's buffer is the same memory) | 8 KB | 16 KB | pool |
| Motion bases, FM6 patches, MIDI, chords, the rest | ~0.5 KB | ~1 KB | RAM |

That is ~11 KB of the 12.2 KB of RAM and ~42 KB of pool against 21.8 KB free. Skiff fits it with "light" T5 / T6 (no
PHYS or GRAIN there: a smaller engine region) and by moving the RAM / POOL split; DIRGE's TAPE, ROOM and VAULT already
sit in that pool. Even trimmed (light engines, no STUT on T5 / T6) it takes nearly all of the free memory, which the
roadmap's own DSP (delay lines, an FDN reverb, chaos oscillators) needs. Two more tracks on one core also cost CPU:
DRONE SUPER, VAULT and TAPE on six tracks wants the second core.

**The project format.** FUN10 is one 3840-byte storage object (`storage.c` ST_PAYLOAD_MAX), full for track parameters
since DUCK. Six tracks of steps alone (6 x 691 B) are 4146 B: six tracks need a new format either way, two objects
per project or the steps moved out of the project.

**The pattern pool (step 6)** is that move: Skiff keeps 64 patterns in their own flash (BK_REC 888 B each, 20 sectors,
80 KB with its crash-safe commits), and a project keeps each track's list A..P of pattern ids, so a project shrinks and
the track count stops driving its size. In DIRGE it needs 80 KB of flash with no free run of that size: the obvious
source is one of the three user sample slots (80 KB each, 0xC8000..0xDC000). It would also let the song chain's 14 KB
of RAM go (patterns read from flash instead of four slots held in RAM).

**Decisions needed before building:**
1. Six tracks at all, given the memory above (recommendation: not now; revisit after DIRGE's own DSP has taken its
   share, and with light T5 / T6 if so).
2. The pattern pool: worth a sample slot (3 to 2)? And a new project format: old FUN10 projects import (their four
   slots become pool patterns, the chain becomes lists), which is the plan's proposal.

## Feature matrix (checked)

Status: **Have** = in DIRGE · **Port** = bring Skiff code in · **Design** = rebuild from GR!MM's idea · **Build** = new
work

| Area | DIRGE today | Skiff | GR!MM | Action |
| --- | --- | --- | --- | --- |
| Home screen | Felucca home | HOME landing page, all 6 tracks | direct access to parameters, doubles as mixer | **Design** (step 8) |
| Tracks / voices | 4 tracks, 8 shared voices, 12 on two cores (done) | 6 tracks (T5 / T6 light), 8 voices, 12 on two cores | 4 tracks, 6 voices | **Port** the 12-voice budget (4); six tracks after a design note (5) |
| Tape FX | TAPE, ROOM, WEAR | — | TAPE | **Have** |
| Long reverb | VAULT ~32 s + INF | ROOM, SPRING, stereo HALL | SPACE ~32 s + INF | **Have** |
| Delay | Felucca delay + WEAR | delay send | tape echo | **Have** |
| Grain | GRAIN engine | GRAIN engine | GRAIN send with FREEZE | **Design** (7) |
| Distortion | DIST (soft clip) + INSERT: SOFT HARD FOLD FUZZ CRUSH | DRIVE: SOFT HARD FOLD BITS + TONE | — | **Have**; DRIVE kinds optional (10) |
| Slicer | SLICER | SLICER (same code) | — | **Have** |
| Sidechain | FX > DUCK (from Skiff) | DUCK | — | **Done** (1) |
| Scales | Felucca's + PHDOM HUNG DHARM | 16 scales | scale snapping | **Have** |
| Envelope | ENV, ENV DEST, ENV SYNC | + ENV 2, a graph | envelope | **Have**; ENV 2 optional (2) |
| LFO | LFO, LFO 2, LFO DEST, MOD | the same four | per-track LFO | **Have** |
| Mixer (GLO) | GLO layer (mute / solo), MIXER page | fuller GLO: FILL, tap tempo | song mode | compare on the unit, then port what is missing |
| Arp | 14 modes, CHANCE, SWING, HOLD | the same | arp + latch | **Have** |
| Chords | SCL > CHORD one-key chords (9 + OFF, 5 voicings) + KEYS LAYER / SPLIT | 9 chords, 5 voicings | one-key chords (hold SEL) | **Have**; GR!MM's gesture (7) |
| Drone latch | ARP HOLD | ARP HOLD | latch | **Have** for the arp; a note latch (7) |
| Sequencer pages | STEP, DETAIL, AUTOMATION, SONG | + ROLL, CHANCE, PLAY, PATTERN | pattern screen | **Port** pages with the pool (6) |
| Automation | SEQ > AUTOMATION | the same | — | **Have** |
| Song mode | Felucca song chain | 64-pattern pool, lists A..P, scenes | slots A..D | **Port** after a design note (6) |
| Performance | PERFORM (FX layer, throws) | PERFORM, 16 scenes | freeze, throws, tape stop, stutter | **Have**; scenes come with the pool (6) |
| Undo / TAKE | 8-level undo, TAKE (from Skiff) | 8-level undo, TAKE | — | **Done** (3) |
| Noise | NOISE engine + DIRGE noise DSP | NOISE engine | — | **Have** (the same Felucca-Engines NOISE) |
| Drums | DRUM | DRUM + DRUM-X | — | optional **Port** as engine 15 (8) |
| Themes | BLOOD CRYPT NEON | SKIFF HI-CON PAPER CRYPT NEON QUARK | pixel-brutalist | **Build** plague doctor (9) |
| Web editor / emulator | yes | yes | yes | **Have** |

## Open questions

- [ ] Six tracks: is the RAM / POOL move worth it, given VAULT and TAPE live in the pool? (Answer with a design note
      and a CPU / memory test before code.)
- [ ] Pattern pool: where does it go in flash, and how many patterns fit beside DIRGE's store, autosave and FM6 bank?
- [ ] Project format: a new DIRGE format with FUN10 import (proposed), or reading Skiff's SKF2 too?
- [x] Skiff DIST vs DIRGE's own dist / fold / crush: neither is needed first; INSERT already has HARD, FOLD, FUZZ
      and CRUSH. Skiff's DRIVE kinds stay optional (10).
- [x] Skiff NOISE vs DIRGE noise: the same Felucca-Engines engine; DIRGE's noise DSP is a separate layer. Keep both.
- [ ] Mixer controls on both HOME and GLO: pick one home for each control.
- [ ] Final firmware name before the first public release.
