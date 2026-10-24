# DIRGE

[![License: GPL-3.0-only](https://img.shields.io/badge/license-GPL--3.0--only-blue.svg)](LICENSE)

**Darksynth, noise and drone firmware for the M-VAVE FM-1.**

**[Try it in your browser](https://sinscum.github.io/DIRGE/)** (the real firmware, no FM-1 needed) ·
**[Install it](https://sinscum.github.io/DIRGE/webapp/installer/)** ·
**[Open the editor](https://sinscum.github.io/DIRGE/webapp/editor/)** (edit sounds live, load samples, back up your
FM-1) · **[Read the manual](MANUAL.md)**

DIRGE turns the FM-1 into a four-track instrument for drones, aggressive mono basses, noise and gritty sequences:
fourteen engines, a 64-step sequencer per track with chords, ratchets, conditions and parameter locks, an arpeggiator,
a song chain, a performance layer, MIDI clock in for an Elektron rig, and long, worn, dark effects over all of it.

It is built on [Felucca](https://github.com/hugelton/Felucca) 1.6.1 by Leo Kuroshita, Hügelton Instruments, and keeps
everything Felucca is. DIRGE adds the engines, effects, scales and look for the noise and darksynth corner other FM-1
firmwares leave open.

Free and open source (GPL-3.0-only). Unofficial firmware, not made or endorsed by M-VAVE or Hügelton Instruments.
**Install at your own risk.**

---

## What's inside

- **4 tracks**, 8 voices shared between them (12 with the second core on, beta); per track MONO, LEGATO, UNISON and glide
- **15 engines:** DRONE and BASS (DIRGE's), ANALOG, PHASE, LOFI, SAMPLE, VOICE, TRIO, WHEEL, GRAIN, PHYS, DRUM, NOISE, FM6,
  SLICE
- **Effects:** per-track DIST, INSERT (fold, crush, fuzz, phaser, ...), SLICER (a tempo-synced gate and stutter) and
  DUCK (sidechain); shared DELAY, CHORUS and REVERB
  (ROOM, SPRING, HALL, and DIRGE's **VAULT**); DIRGE's **TAPE** page over the whole mix
- **Per track:** envelope, two LFO pages, a 4-slot modulation matrix, scale and chord keys, arpeggiator (14 modes)
- **Sequencer:** 64 steps per track, chance, ratchets, trig conditions, nudge, parameter locks, automation, step tools
  (euclid, random, reverse, shift, double, half), a drum grid
- **Song chain** of patterns, and a **performance layer**: repeat, reverse, tape stop, freeze, octave shifts, filter
  sweeps, reverb and echo throws
- **MIDI:** in over USB and TRS, out over USB, MIDI clock in (slaves to a Digitakt II / Digitone II), MIDI learn, a channel
  per track; the FM-1 is also a USB audio input for your computer
- **Projects, user presets, phrases and autosave:** the FM-1 comes back as you left it

### What DIRGE adds to Felucca

- **BASS engine**: the darksynth mono bass. SAW / SQR / PULSE / DUO, a detuned second oscillator, a square sub, drive
  into a 24 dB resonant low-pass with its own velocity-driven envelope. Sounds: PERTURB, BRUT, GRAVE, SIREN, HEARSE,
  BONESAW.
- **DRONE engine**, first in the PRESETS list: polyBLEP saws pushed apart by DTN and pulled about by DRFT (one slow
  random walk), a sine SUB, BRTH (noise), DRV (tanh) and a rounded resonant low-pass. WAVE: **DRONE** (two saws at the
  note, two an octave down), **HIGH** or **SUPER** (seven saws at the JP-8000 supersaw's offsets). Sounds: DIRGE,
  BLACK MASS, SHIVER, FURNACE, ASH WIND, GRAVEWAVE, NEON TOMB.
- **FX > TAPE**, three knobs, all off by default:
  - **TAPE:** the whole mix through a worn cassette (wow, flutter, saturation, a high cut, hiss that follows the music)
  - **ROOM:** a dead channel's room tone under the mix (static, 50 Hz hum, crackle), into the reverb too
  - **WEAR:** the delay worn into a tape echo (wow and flutter on its read, darker, saturated repeats)
- **LATCH** (after GR!MM's): GLO + G4 latches a track's keys, so a drone keeps sounding after you let go.
- **TAKE** (from Skiff): play with REC off, then hold REC + C4 to keep the last loop you played (notes, holds, knob
  moves) in the pattern. **8 levels of undo**: hold SAVE, then OCT− / OCT+ to step further back or forward.
- **FX > DUCK** (ported from [Skiff](https://github.com/illuminus5/skiff)): a track ducks under another track's notes
  (a drum track's kick only), DEPTH and RELEASE: the sidechain pump for basses and drones.
- **REVERB TYPE VAULT:** HALL stretched to ~32 s; at SIZE 127 **INF**, a freeze that holds what is in it.
- **Dark scales:** PHDOM (Phrygian dominant), HUNG (Hungarian minor), DHARM (double harmonic).
- **KEYS LAYER / SPLIT** (MENU > AUDIO > KEYS, SPLIT): play a second track from every key, or from the keys under a
  split point.
- **Power-on scenes** (MENU > SYSTEM > SCENE): **DIRGE** (DRONE, a fat bass with a pattern, wind noise and a deep kit
  at 90 BPM), **FELUCCA** or **BLANK**.
- **The look:** four palettes, **BLOOD** (the default), **CRYPT**, **NEON** and **PLAGUE** (the plague doctor's
  leather, parchment and brass), and a splash framed like a coffin lid.
  Felucca's palettes are all still there.

Every DIRGE addition is off, or bit for bit Felucca, until you turn it on: Felucca's projects and sounds open and play
as they did.

## Install

Back up first: the installer saves your FM-1's music, sounds, samples and settings before it changes anything.

**In the browser** (Chrome or Edge on a computer): open the **[DIRGE installer](https://sinscum.github.io/DIRGE/webapp/installer/)**,
connect the FM-1 over USB and follow it. The same page takes the FM-1 back to the official V15 firmware
(**Return to official V15**).

**With Python 3:** download the `.fwsc` package (CI artifacts now, GitHub Releases from the first release), then:

```sh
pip install mido python-rtmidi
python3 tools/fm1_install.py dirge-X.Y.fwsc
```

**If something goes wrong:** hold **OCT−** while switching the FM-1 on for the USB rescue mode, then install again.
Before the first install of any custom firmware, a full flash backup with M-Flasher or MvaveFM1Unbricker is a good
idea.

## Learn it

- **[The manual](MANUAL.md):** every button, page, layer and setting, and what DIRGE adds.
- **Quick start:** switch on. Turn **ALGORITHM** to pick a track, turn **PRESETS** to pick a sound, play the keys.
  Press **PLAY** to hear the power-on scene. Tap a page button (ENV, LFO, FX, SCL, EDIT, ARP, SEQ) for its pages;
  **hold** one for its quick layer. Hold **HOME** for the MENU. Hold **SAVE** to undo (OCT− for further back).
- **MENU > SYSTEM > HELP ON** shows a short hint on every page.

## The editor and the demo

Both run in Chrome or Edge on a computer, nothing to install.

- **[The editor](https://sinscum.github.io/DIRGE/webapp/editor/):** connect the FM-1 over USB and edit every track's
  sound, steps, mixer and automation; every move plays on the FM-1 at once. Browse the factory sounds, keep a library
  of your own, edit FM6 operators, load WAVs into the sample slots, manage projects, and back up or restore
  everything. Without an FM-1, **[try it with a simulated one](https://sinscum.github.io/DIRGE/webapp/editor/?mock=1)**.
- **[The demo](https://sinscum.github.io/DIRGE/):** the FM-1's panel on screen, running the same firmware compiled
  for the browser, with a live oscilloscope. Play it with the mouse, a touch screen or the computer keyboard.

## Your own samples

Open the [editor](https://sinscum.github.io/DIRGE/webapp/editor/), connect the FM-1 and go to **Samples**: drop a WAV
on one of the three user slots (USR1..USR3, about 7 s each) and upload it. Then pick the SAMPLE or SLICE engine on a track. From a command line:

```sh
python3 tools/fm1_sample_upload.py info
python3 tools/fm1_sample_upload.py load 1 GRIT "your-sound.wav:48"   # slot USR1, root note C3
```

## Building from source

Linux x86-64 (or Docker on macOS): the JieLi toolchain and Python 3 with Pillow (with libraqm) and fontTools; the
three JieLi SDK files are in `sdk/ac79`. Full guide: [BUILDING.md](BUILDING.md).

```sh
git clone --recurse-submodules https://github.com/sinscum/DIRGE.git
AC79_SDK=sdk/ac79 ./build.sh              # build/felucca.fwsc: identity FM-1_660 (releases: --release X.Y, FM-1_6XY)
AC79_SDK=sdk/ac79 sh tests/run_tests.sh   # every host test: Felucca's suite and DIRGE's (tests/dirge/)
web/emu/build.sh                          # the browser emulator (Emscripten): build/emu/
```

CI (`.github/workflows/ci.yml`) builds, runs every test, builds the emulator and, on `main`, publishes the site.

## Contributing and roadmap

Bugs and ideas: [Issues](https://github.com/sinscum/DIRGE/issues) (see [CONTRIBUTING.md](CONTRIBUTING.md)); please
report DIRGE's behaviour here, not in Felucca's tracker. Where DIRGE goes next (from
[Skiff](https://github.com/illuminus5/skiff): a 12-voice budget, six tracks and a pattern pool, GR!MM-style
HOME and latch): [docs/MERGE_PLAN.md](docs/MERGE_PLAN.md). The chip's rules and the project's conventions: [CLAUDE.md](CLAUDE.md).

## Upstream

DIRGE is a fork with Felucca's history in it (`git remote add felucca https://github.com/hugelton/Felucca.git`), so
Felucca's fixes merge in. Felucca's author has stepped back from active development (occasional maintenance);
[Felucca-Engines](https://github.com/hugelton/Felucca-Engines), the MIT engines in `firmware/engines/` (a submodule),
is still maintained. DIRGE's changes to Felucca's own files are kept small and marked `DIRGE`.

## Credits and licence

GPL-3.0-only, as Felucca: [LICENSE](LICENSE), with the credits and third-party parts in [LICENSING.md](LICENSING.md)
and [CREDITS.md](CREDITS.md).

- **[Felucca](https://github.com/hugelton/Felucca)** and **[Felucca-Engines](https://github.com/hugelton/Felucca-Engines)**
  by Leo Kuroshita, Hügelton Instruments: the platform, the engines, the editor, installer and emulator
- **[GHOULBOX](https://github.com/jasonpersinger/ghoulbox-fm1-dungeon-synth)** by jasonpersinger: TAPE (ported),
  ROOM and KEYS after its tape, cave ambience and keyboard layer
- **[GR!MM](https://github.com/GRiMMBIENT/grimm)** by GRiMMBIENT: VAULT and DRONE's SUPER after its long reverb and
  supersaw; the shape of this README and the manual
- **[Skiff](https://github.com/illuminus5/skiff)** by illuminus5: the reference for the features planned next

"M-VAVE" and "FM-1" are trademarks of their respective owners; DIRGE is not affiliated with or endorsed by them. Like
Felucca, DIRGE never enables the hardware's Bluetooth / Wi-Fi radio.
