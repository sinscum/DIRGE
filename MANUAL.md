# DIRGE manual

For the M-VAVE FM-1 · DIRGE on Felucca 1.6.1

This manual is for playing DIRGE. It covers the controls, every page, the quick layers, the MENU and what DIRGE adds to
Felucca. To build it or change it, see [BUILDING.md](BUILDING.md) and [CLAUDE.md](CLAUDE.md).

**Contents:** [1. The panel](#1-the-panel) · [2. The basics](#2-the-basics) · [3. HOME](#3-home) ·
[4. Sounds: PRESETS and EDIT](#4-sounds-presets-and-edit) · [5. The engines](#5-the-engines) ·
[6. ENV and LFO](#6-env-and-lfo) · [7. FX](#7-fx) · [8. SCL: scales and chords](#8-scl-scales-and-chords) ·
[9. ARP](#9-arp) · [10. The sequencer](#10-the-sequencer) · [11. GLO: song and mixer keys](#11-glo-song-and-mixer-keys) ·
[12. SAVE](#12-save) · [13. Quick layers](#13-quick-layers) · [14. Performance](#14-performance) ·
[15. The MENU](#15-the-menu) · [16. MIDI and clock](#16-midi-and-clock) · [17. The editor and the demo](#17-the-editor-and-the-demo) ·
[18. Install, update, rescue](#18-install-update-rescue) · [19. Combos at a glance](#19-combos-at-a-glance)

---

## 1. The panel

![The FM-1's controls](docs/controls.jpg)

| Control | What it does |
|---|---|
| **ALGORITHM** knob | Picks the track you are working on (T1–T4), on every page. In the MENU: the tab. |
| **PRESETS** knob | Loads the next or previous sound into the selected track. In the MENU and on lists: the row. |
| **SELECT** knob | The tempo (by default). MENU > CONTROL > SELECT makes it step the pages, or play mod wheel or velocity. |
| **KNOB 1–4** | The four values at the bottom of the screen, whatever page you are on. |
| **MASTER** knob | Volume. |
| **OCT− / OCT+** | Octave down / up. On action pages and in dialogs: back / do it. |
| **Page buttons** | HOME, ENV, LFO, FX, SCL, EDIT, GLO, SAVE, ARP, SEQ (below). |
| **PLAY / REC** | Start and stop; arm a track for recording. |
| **Keys** | Play the selected track. While a layer button is held: that layer's shortcuts. |

## 2. The basics

DIRGE has **four tracks (T1–T4)**. Each plays one engine with its own sound, envelope, LFO, effects sends, scale,
arpeggiator and 64-step pattern. The four share **eight voices**, or **twelve** with MENU > SYSTEM > 2ND CORE ON
(beta: the FM-1's second processor core renders some of the tracks; it takes effect from the next power-on).

**Tap, hold, double-tap.** Tap a page button to open its first page; tap it again for its next page (the pages go
round). **Hold** a page button to open its **quick layer**: the keys and knobs become its shortcuts while you hold it
([13](#13-quick-layers)). **Double-tap** it to lock the layer open without holding.

**Knobs speed up:** turn slowly for fine steps, quickly to sweep (MENU > CONTROL > KNOB ACCEL).

**Undo:** hold **SAVE**. It undoes the last sound load, clear, sequence tool, take or recording; hold it again to
redo. DIRGE keeps the last **8** changes: for 3 seconds after an undo, **OCT−** goes one further back and **OCT+** one
forward again (the header says how far: UNDO T2 (3/8)). After that OCT− and OCT+ are the octave again.

**Momentary edits:** hold **LFO** and turn knobs on HOME, a sound page or an FX page; let go and every value snaps
back. Press OCT+ while still holding LFO to keep the changes.

**Autosave:** your work comes back at power-on (MENU > SYSTEM > RESTORE LAST). DIRGE saves only while it is silent, to
avoid audio dropouts.

**HELP:** MENU > SYSTEM > HELP ON shows a two-second hint (keycaps and a few words) each time you open a page or a
layer.

**At power-on** DIRGE starts with a **scene** (MENU > SYSTEM > SCENE):

| Scene | What plays |
|---|---|
| **DIRGE** (default) | T1 DRONE, T2 TRIO's FAT BASS with a pattern, T3 NOISE's WIND, T4 DRUM's DEEP KIT; 90 BPM |
| **FELUCCA** | Felucca's own power-on |
| **BLANK** | Empty tracks at 90 BPM |

With RESTORE LAST on, your autosaved work replaces the scene.

## 3. HOME

The main screen: the selected track and its sound in the header, and a view below it (MENU > DISPLAY > HOME):
**SCOPE** (the output's waveform), **TRACKS** (the four tracks) or **LEVELS** (four level faders).

**KNOB 1–4 on HOME** are the selected engine's four main controls (DRONE: DTN, CUT, SUB, DRFT). On the LEVELS view
they are the four tracks' levels.

Tap **HOME** again for:

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| **MIXER** | LEVEL | PAN | REV send | MUTE |
| **CLOCK** | BPM | SWING | CLK: INT / USB / TRS | |

On MIXER, ALGORITHM picks the track. **Hold HOME** for the [MENU](#15-the-menu).

## 4. Sounds: PRESETS and EDIT

**Loading a sound:** turn **PRESETS** anywhere. The steps of the track stay; only the sound changes. Hold SAVE to undo.

**SAVE > PRESETS** is the browser: KNOB 2 the engine, KNOB 3 favourites, KNOB 4 the list. DRONE is first.

**EDIT** opens the selected track's sound:

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| **EDIT 1** | the engine's first four parameters | | | |
| **EDIT 2** | the engine's next four | | | |
| **VOICE** | VOICE: POLY / MONO / LEG / UNI | GLIDE | GLIDE MODE: RATE / TIME | PRIORITY: LAST / LOW / HIGH |
| **VOICE 2** | ALLOC | DETUNE | PAN | MUTE |
| **VOICE 3** | SPREAD (the voices around PAN) | | | |

Engine-specific pages join EDIT where they apply: **FILTER** (ANALOG: LP / BP / HP, cutoff, resonance, key track),
**LANES** and **LANES 2** (DRUM: the eight lane levels), **SLICES** (SLICE: move, split, join slices by hand),
**OPERATOR**, **OP ENV**, **OPERATOR 2** (FM6's operators, byte by byte).

**MONO** plays one voice, **LEG** (legato) glides between overlapping notes without retriggering, **UNI** stacks
detuned voices on one note: the darksynth bass and lead settings.

**Hold EDIT** to switch engines from the keys ([13](#13-quick-layers)).

## 5. The engines

| Engine | What it is |
|---|---|
| **DRONE** (DIRGE) | Detuned polyBLEP saws for drones, pads and supersaw leads ([below](#drone)) |
| **ANALOG** | Virtual analog: oscillators into a state-variable filter (LP / BP / HP) |
| **PHASE** | CZ-style phase distortion |
| **LOFI** | Crushed, aliased digital tones |
| **SAMPLE** | The built-in CC0 sample sets and your three user slots |
| **VOICE** | A formant voice and choir |
| **TRIO** | Three oscillators: basses, leads |
| **WHEEL** | Tonewheel organ |
| **GRAIN** | Granular clouds |
| **PHYS** | Physical models: strings, plates, bowed and struck |
| **DRUM** | An eight-lane kit (KICK .. BELL), on the GM map; with a drum grid on SEQ |
| **NOISE** | Noise instruments: wind, surf, static, hits |
| **FM6** | Six-operator FM (DX7-style), editable on the unit and in the editor |
| **SLICE** | A sample cut into slices across the keys |

### DRONE

Saws pushed apart by **DTN** and pulled about by **DRFT** (one slow random walk; each saw follows it its own way), a
sine **SUB**, **BRTH** (breath noise), **DRV** (tanh drive) and a rounded resonant low-pass that keeps DC out of a long,
low drone. **WAVE:** **DRONE** (two saws at the note, two an octave down), **HIGH** (the upper pair alone) or **SUPER**
(seven saws at the JP-8000 supersaw's offsets, DTN their spread). Up to four voices.

Sounds: **DIRGE**, **BLACK MASS**, **SHIVER**, **FURNACE**, **ASH WIND**, **GRAVEWAVE** (a supersaw pad), **NEON TOMB**
(a mono darksynth lead).

## 6. ENV and LFO

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| **ENV** | ATTACK | DECAY | SUSTAIN | RELEASE |
| **ENV DEST** | to FILTER | to PITCH | to SHAPE | ENV SYNC (times as note values) |
| **LFO** | RATE | WAVE: SIN TRI SAW SQR S&H | PHASE | FADE |
| **LFO 2** | SYNC (OFF or a note value) | TRIG: NOTE / FREE | POL: BI / UNI | |
| **LFO DEST** | to PITCH | to FILTER | to SHAPE | to AMP |
| **MOD** | the slot (1–4) | SOURCE | DEST | AMOUNT |

**MOD** is the modulation matrix: four slots per track, each a source (LFO, ENV, VEL, KEY, RAND, MODW, AT, EXPR,
S&H, SLEW) to a destination (PITCH, CUT, SHP, AMP, PAN, DIST, the sends, the LFO's RATE, VIB, the engine's E1–E8, ...)
with an amount. S&H on the LFO with a slow rate is the slow random modulation for drones.

## 7. FX

Tap **FX** to go round its pages. The first five are the selected track's; the rest are shared by all tracks.

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| **FX** (track) | DIST | CHORUS send | DELAY send | REVERB send |
| **SLICER** (track) | OFF / GATE / STUT | PATTERN (16) | RATE: 1/8 1/16 1/32 8T 16T 32T | DEPTH |
| **INSERT** (track) | TYPE | A | B | C |
| **INSERT 2** (track) | MIX | | | |
| **DUCK** (track, DIRGE) | BY: OFF / T1–T4 | DUCK (the depth) | RLS (the release) | |
| **DLY** (all) | TIME (note values, dotted too) | FEEDBACK | COLOR | MIX |
| **REVERB** (all) | TYPE: ROOM / SPRING / HALL / **VAULT** | SIZE | DAMP | |
| **CHORUS** (all) | RATE | DEPTH | | |
| **TAPE** (all, DIRGE) | **TAPE** | **ROOM** | **WEAR** | |

- **DIST:** a low cut, then drive into an asymmetric soft clip, then a tone filter that closes as it drives.
- **INSERT TYPE:** OFF, SOFT, HARD, FOLD (wavefolder), FUZZ, CRUSH (bits and sample rate), PHASR, FLANG, CHOR. The
  distortion chain for basses and leads: try FOLD or CRUSH with MIX at full.
- **SLICER:** chops the track into a tempo-synced rhythm. GATE closes the sound on the pattern's off steps; STUT
  repeats the last on step instead. It locks to the sequencer and swing.
- **DUCK** (DIRGE, from Skiff): the sidechain pump. The track dips each time the track set in **BY** plays a note,
  by DUCK (100 % is silent), and comes back over RLS (the release). A DRUM track ducks others only with its KICK lane, so the
  hats never pump. Put a drone or a bass on DUCK BY the drum track for the darksynth pump. It works after the track's
  DIST, INSERT and SLICER and before its level and sends, so the sends dip too while the delay and reverb tails ring
  on. A sound load keeps DUCK (it belongs to the track's mix); DUCK and RLS can be automated and locked per step.
  BY skips the track itself.
- **VAULT** (DIRGE): HALL stretched to about 32 seconds. At **SIZE 127** it becomes **INF**: a freeze that holds what
  is in the reverb while you play over it.
- **TAPE** (DIRGE), all three off by default:
  - **TAPE:** the whole mix through a worn cassette: wow, flutter, saturation, a high cut, and hiss that follows the
    music.
  - **ROOM:** a dead channel's room tone under the mix: static, 50 Hz hum, crackle. It goes into the reverb too.
  - **WEAR:** the delay worn into a tape echo: wow and flutter on its read, darker, saturated repeats.

## 8. SCL: scales and chords

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| **SCL** | ROOT | SCALE | QUANTIZE | TRANSPOSE |
| **CHORD** | CHORD | VOICING | | |

**Scales:** CHR, MAJ, MIN, DOR, MIX, PEN, MPEN, HARM, PHRY, LYD, LOC, MEL, BLUES, WHOLE, DIMHW, DIMWH, and DIRGE's
**PHDOM** (Phrygian dominant), **HUNG** (Hungarian minor) and **DHARM** (double harmonic). QUANTIZE snaps the keys to
the scale.

**One-key chords:** set CHORD (DIA3, DIA7 follow the scale; MAJ, MIN, DOM7, MAJ7, MIN7, SUS4, POW are fixed shapes)
and VOICING (CLOSE, OPEN, INV1, INV2, +OCT): every key then plays a chord. MONO tracks play the root.

**Hold SCL** and press a key to set the root ([13](#13-quick-layers)).

## 9. ARP

| Page | KNOB 1 | KNOB 2 | KNOB 3 | KNOB 4 |
|---|---|---|---|---|
| **ARP** | MODE | RATE | OCTAVES | GATE |
| **ARP 2** | SWING | CHANCE | HOLD (latch) | ORDER |

**Modes:** OFF, UP, DN, UPDN, RND, ORD, REPEAT, DNUP, UP+8, CONV, DIVG, PINKY, THUMB, WALK, CHORD. **HOLD** keeps the
arpeggio going after you let go: a latch for drones and long patterns.

## 10. The sequencer

Each track has a pattern of up to **64 steps**.

| Page | What it does |
|---|---|
| **STEP** | The steps on a roll. KNOB 1 moves the cursor; the keys write notes (and chords) into the step. |
| **DETAIL** | The cursor step's CHANCE (and conditions past 100 %: 1:4, FILL, ...), RATCHET, NUDGE, VELOCITY. PRESETS steps through the steps. |
| **AUTOMATION** | The track's parameter locks and recorded knob moves as a list: pick a row, OCT+ to act, EDIT to delete. |

**Recording live:** press **REC** to arm the track, **PLAY** to start: the keys and knob moves you play go into the
pattern. **Hold REC** for its layer: CLEAR (the track's steps and automation), CLICK (OFF / REC / ON), COUNT-IN
(OFF / 1 BAR / 2 BARS), the click level, and **TAKE** (C4).

**TAKE** (DIRGE, from Skiff): you don't have to arm REC to keep a good idea. While the sequencer plays, DIRGE quietly
logs what you play on each track (the keys, MIDI, the arpeggiator) and your knob moves. **Hold REC and press C4** to
put the last complete loop you played on the selected track into its pattern, as REC would have recorded it: notes
into their steps (with their timing), held notes tied, knob moves as automation. It adds to what is there and never
wipes a note. Hold SAVE to undo it. The REC map's TAKE row says if it can't yet (PLAY A LOOP, DISARM REC, STOP TO
EDIT). The log starts fresh at each PLAY and holds about 128 notes; a busier loop is taken in part (TAKE PARTIAL).

**Parameter locks:** open the sound page you want (HOME, ENV, LFO, FX, EDIT, ...), then on SEQ > STEP hold a step
and turn KNOB 1–4: that step plays the locked value of the knob's parameter on that page. Tap EDIT while holding the
step to clear its locks. Locks show up in AUTOMATION.

**Hold SEQ** for the sequence tools on the white keys: CLEAR, REVERSE, SHIFT < >, RANDOM, COOK, EUCLID, DOUBLE,
HALF; KNOB 1–4 set LENGTH, DIVISION, SWING, GATE. On a DRUM track the black keys pick the lane and BEAT fills a
starting pattern. Each tool is one undo.

**The DRUM grid:** on a DRUM track SEQ shows the kit's lanes; the keys toggle steps, KNOB 2 picks the lane.

## 11. GLO: song and mixer keys

Tap **GLO** for **SONG**, the song chain: KNOB 1 picks the section, the keys A–D pick patterns, PLAY plays the song.

**Hold GLO** for the mixer keys ([13](#13-quick-layers)): mutes, solos, FILL, tap tempo and the four levels.
GLO + PLAY restarts from the top without stopping.

## 12. SAVE

Tap **SAVE** to go round:

| Page | What it does |
|---|---|
| **USER** | 32 user presets: KNOB 1 the slot, OCT+ saves the selected track's sound. EDIT names it. |
| **PRESETS** | The browser (above). |
| **PHRASES** | Load a ready-made pattern into the track: KNOB 1 the phrase, OCT+ loads. |
| **PROJECT** | Projects: KNOB 1 the slot, KNOB 3 LOAD, KNOB 4 SAVE, KNOB 2 ERASE; EDIT names it. |
| **TOOLS** | CLEAR the sequence, INIT or CLEAN INIT the sound; OCT+ does it. |

Saving is refused while the sequencer plays (stop it first). **Hold SAVE** anywhere to undo.

## 13. Quick layers

Hold a page button (or double-tap it to lock it) and its map shows over the page. Keys pressed in a layer play no
notes and record nothing.

| Hold | White keys from F3 | Black keys | KNOB 1–4 |
|---|---|---|---|
| **FX** | the performance effects (below) | T1–T4 mute while held | macros |
| **GLO** | F3–B3 solo T1–T4 (while held), C4 unmute all, D4 MIDI LEARN, E4 FILL, F4 tap tempo | T1–T4 mute (latched) | T1–T4 level |
| **SCL** | any key: the root | | ROOT, SCALE, CHORD, VOICING |
| **EDIT** | one engine per key, then INIT (E5) and CLEAN INIT (F5) | the engine's quick actions (DRUM: lane mutes) | ENGINE, SOUND, FAV |
| **SEQ** | the sequence tools | DRUM lanes | LENGTH, DIV, SWING, GATE |
| **REC** | CLEAR, CLICK, COUNT-IN, CLICK LEVEL, TAKE (C4) | | CLICK, COUNT-IN, LEVEL |

In the SET layers (GLO, SCL, EDIT, SEQ, REC), **OCT−** puts back whatever the layer changed since it opened.

## 14. Performance

**Hold FX** (or double-tap to lock) and hold a white key: its effect plays on the whole mix until you let go.

The default keys: **REPEAT 1/8, 1/16, 1/32**, **REVERSE**, **LPF**, **HPF**, **TAPE STOP**, **FREEZE**, **OCT UP**,
**OCT DN**, **FLANGER**, **PHASER**, **REV THROW**, **ECHO THROW**. REPEAT and REVERSE start on the next 1/16. Throws
push every track's reverb or delay send to full while held, then let the tail ring.

Any effect can go on any key: in the layer, hold a white key and turn PRESETS; EDIT puts the key's default back.

## 15. The MENU

Hold **HOME**. ALGORITHM picks the tab, PRESETS the row, any of KNOB 1–4 or OCT± the value. HOME leaves.

| Tab | Rows |
|---|---|
| **DISPLAY** | COLOR (BLOOD, CRYPT, NEON and Felucca's palettes), STYLE (FLAT / LINE), LARGE, ANIM, LEDS, SCREEN OFF, SCOPE, HOME (SCOPE / TRACKS / LEVELS) |
| **CONTROL** | HOLD (layer timing), KNOB ACCEL, FX LATCH, BPM LOCK, SCALE LEDS, STEP PREVIEW, CHORD ENTRY, SELECT (BPM / PAGE / MODW / VEL) |
| **AUDIO** | SPEAKER EQ, USB LEVEL, CLICK, CLICK LEVEL, COUNT-IN, TUNE, **KEYS**, **SPLIT** |
| **MIDI** | MIDI IN, T1–T4 CHANNEL, AUTO SELECT, SCREEN WAKE, LEARN CLEAR |
| **SYSTEM** | USB SERIAL, RESTORE LAST, HELP, **SCENE**, 2ND CORE (beta; ON: 12 voices instead of 8), CALIBRATION, INFO, ABOUT |

**KEYS / SPLIT** (DIRGE, saved with the project): **LAYER** plays the selected track and a second track from every
key; **SPLIT** plays a second track from the keys under the split point (C4 by default) and the selected track above
it.

**ABOUT** shows the version: give it when you report a bug.

## 16. MIDI and clock

- **Clock:** HOME > CLOCK > CLK: **INT** (DIRGE's own tempo), **USB** or **TRS** (follow an external MIDI clock, start
  and stop).
- **With a Digitakt II / Digitone II:** connect the Elektron's MIDI out to the FM-1's TRS MIDI in (or both to a
  computer over USB), turn on the Elektron's clock and transport send, and set CLK to TRS (or USB). Set the track
  channels in MENU > MIDI.
- **Notes in:** MENU > MIDI > MIDI IN routes channels to tracks: CH1-4 (channels 1–4 to T1–T4), SEL (every channel
  to the selected track), CH5-8, CH9-12, CH13-16, or TRACK (each track its own channel, set in T1–T4 CHANNEL). AUTO
  SELECT makes incoming notes select their track.
- **MIDI learn:** hold GLO, press D4, turn a knob, then move a CC on your controller. LEARN CLEAR forgets them.
- **Out:** the notes you play on the keys go out over USB MIDI, on each track's channel.
- **USB audio:** the FM-1 appears as a USB audio input on your computer (MENU > AUDIO > USB LEVEL).

## 17. The editor and the demo

**[The editor](https://sinscum.github.io/DIRGE/webapp/editor/)** (Chrome or Edge on a computer): connect the FM-1 and
press Connect. Every change plays on the FM-1 at once.

- **Sound:** every parameter of every track, the envelopes and LFOs drawn, the modulation matrix
- **FM6:** the six operators, algorithms, DX7 SysEx import and the voice bank
- **Sequencer:** the steps, locks and automation; **Tracks:** the mixer
- **Library:** the factory sounds, and your own kept on the computer
- **Samples:** drop a WAV on a user slot (USR1–USR3, about 7 s each) and upload it
- **Projects:** load, save, name; **Settings:** the MENU's settings, a complete **backup** and **restore**

No FM-1 at hand? **[Open the editor with a simulated FM-1](https://sinscum.github.io/DIRGE/webapp/editor/?mock=1).**

**[The demo](https://sinscum.github.io/DIRGE/)** runs the same firmware in the browser on a drawn FM-1, with a live
oscilloscope: play it with the mouse, a touch screen or the computer keyboard. Its saves stay in that browser.

## 18. Install, update, rescue

- **Install / update:** the [installer](https://sinscum.github.io/DIRGE/webapp/installer/) (Chrome or Edge), or
  `python3 tools/fm1_install.py dirge-X.Y.fwsc` (needs `pip install mido python-rtmidi`). Your projects, presets,
  samples and settings stay.
- **Back to the official firmware:** the installer's **Return to official V15** saves a complete backup first.
- **Rescue:** hold **OCT−** while switching on for USB update mode, then install again. If the FM-1 does not start at
  all, [FM-1-transporter](https://github.com/kurogedelic/FM-1-transporter) brings it back. Before the first install of
  any custom firmware, back up the whole flash with M-Flasher or MvaveFM1Unbricker.

## 19. Combos at a glance

| Combo | Does |
|---|---|
| Hold **HOME** | MENU |
| Hold **SAVE** | Undo (again: redo); then OCT− / OCT+ for 3 s: further back / forward, up to 8 |
| Hold **LFO** + knobs | Momentary edits (OCT+ keeps them) |
| Hold **FX** + white key | A performance effect |
| Hold **FX** + black key | Mute T1–T4 while held |
| Hold **GLO** + black key | Mute T1–T4 (latched) |
| Hold **GLO** + F3–B3 | Solo T1–T4 while held |
| Hold **GLO** + D4 | MIDI LEARN |
| Hold **GLO** + E4 / F4 | FILL / tap tempo |
| **GLO** + **PLAY** | Restart from the top |
| Hold **SCL** + key | Set the root |
| Hold **EDIT** + white key | Switch engine |
| Hold **SEQ** + white key | A sequence tool |
| Hold **REC** + white key | CLEAR, CLICK, COUNT-IN, CLICK LEVEL, TAKE (C4) |
| Double-tap a layer button | Lock its layer open |
| Hold **OCT−** at power-on | USB rescue |
