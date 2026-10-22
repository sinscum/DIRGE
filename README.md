# DIRGE

[![License: GPL-3.0-only](https://img.shields.io/badge/license-GPL--3.0--only-blue.svg)](LICENSE)

**Darksynth, noise and drone firmware for the M-VAVE FM-1**, built on [Felucca](https://github.com/hugelton/Felucca)
1.6.1 by Leo Kuroshita, Hügelton Instruments. DIRGE keeps everything Felucca is (the engines, the voices with MONO,
LEGATO, UNISON and glide, the sequencer and song chain, the modulation matrix, MIDI clock in over USB and TRS, the
effects, the projects and presets, the web editor and installer) and adds its own engines and effects for the noise
and darksynth corner that other FM-1 firmwares leave open.

Try it in your browser, install it and edit it from Chrome or Edge: <https://sinscum.github.io/DIRGE/>
(the installer: `webapp/installer/`, the editor: `webapp/editor/`). Installing is at your own risk; the installer's
**Return to official V15** takes you back.

## What DIRGE adds

- **Power-on scenes** (MENU > SYSTEM > SCENE): **DIRGE** by default (DRONE on T1, TRIO's FAT BASS with a pattern on T2,
  NOISE's WIND on T3, DRUM's DEEP KIT on T4; 90 BPM), **FELUCCA** (Felucca's own power-on) or **BLANK**.
- **DRONE** (`firmware/src/eng_drone.c`), first in the PRESETS engine list: polyBLEP saws pushed apart by DTN and
  pulled about by DRFT (one slow random walk, each saw following it its own way), a sine SUB, BRTH (noise), DRV
  (tanh) and a rounded resonant low-pass. WAVE: **DRONE** (two saws at the note, two an octave down), **HIGH** (the
  upper pair alone) or **SUPER** (seven saws at the JP-8000 supersaw's offsets, DTN their spread). HOME's knobs: DTN,
  CUT, SUB, DRFT. Presets DIRGE, BLACK MASS, SHIVER, FURNACE, ASH WIND, GRAVEWAVE (a supersaw pad) and NEON TOMB (a
  mono darksynth lead). Up to 4 voices.
- **FX > TAPE**, a page of three knobs, all off by default:
  - **TAPE**: the whole mix through a worn cassette (wow, flutter, saturation, a high cut, hiss that follows the music).
  - **ROOM**: a dead channel's room tone under the mix (static, 50 Hz hum, crackle), into the reverb too.
  - **WEAR**: the delay worn into a tape echo (wow and flutter on its read, darker and saturated repeats).
- **REVERB TYPE VAULT**: HALL stretched to ~32 s, and at SIZE 127 **INF**, a freeze that holds what is in it.
- **Dark scales**: PHDOM (Phrygian dominant), HUNG (Hungarian minor), DHARM (double harmonic).
- **KEYS LAYER / SPLIT** (MENU > AUDIO > KEYS, SPLIT; saved with the project): LAYER plays the selected track and a
  second one from every key; SPLIT plays a second track from the keys under the split point (C4 by default).
- **DIRGE's DSP** (`firmware/src/dirge/`), beside Felucca-Engines' `dsp.c`: the rounded SVF low-pass (no DC creeping
  into a long low drone), white, pink, brown, crackle and shift-register noise, the tape and the room tone.
- **The look**: three palettes, **BLOOD** (the default: black, bone and blood red, a ghost-cyan cursor), **CRYPT**
  (grave violet, toxic green) and **NEON** (an 80s horror poster's magenta and cyan) in MENU > DISPLAY > COLOR, and a
  splash framed like a coffin lid. Felucca's palettes are all still there.
- **The site**: the emulator in DIRGE's dead-channel room, with a live oscilloscope of its output.

TAPE, ROOM and WEAR use three global ids that Felucca's projects never set (the old GM drum part's level and reverb,
and an unused editor id), so every Felucca project opens with them off and plays as it did. KEYS uses two spare bytes
of Felucca's project format; a project without it is byte for byte what Felucca writes.

Next: distortion, fold and crush; chaos and feedback oscillators; a mono darksynth bass engine; slow random
modulation. See [CLAUDE.md](CLAUDE.md) for the plan and the chip's rules.

## Build

Linux x86-64 (or Docker on macOS: see [BUILDING.md](BUILDING.md), Felucca's guide, which applies as it is). The
JieLi toolchain and Python 3 with Pillow (with libraqm) and fontTools; the three JieLi SDK files are in `sdk/ac79`:

```sh
git clone --recurse-submodules https://github.com/sinscum/DIRGE.git
AC79_SDK=sdk/ac79 ./build.sh          # build/felucca.fwsc: identity FM-1_660 (releases: --release X.Y, FM-1_6XY)
AC79_SDK=sdk/ac79 sh tests/run_tests.sh   # every host test: Felucca's suite and DIRGE's (tests/dirge/)
web/emu/build.sh                      # the browser emulator (Emscripten): build/emu/
```

The toolchain: `tools/get_toolchain.sh`, or copied out of foma's image
(`docker create konsumer/foma-toolchain`, then `docker cp -L <id>:/opt/jieli/toolchain ~/.jieli/toolchain`), as CI
does (`.github/workflows/ci.yml`: build, tests, emulator, and on `main` the site on GitHub Pages).

## Upstream

DIRGE is a fork with Felucca's history in it (`git remote add felucca https://github.com/hugelton/Felucca.git`), so
Felucca's fixes merge in. Felucca's author has stepped back from active development (occasional maintenance);
[Felucca-Engines](https://github.com/hugelton/Felucca-Engines), the MIT engines in `firmware/engines/` (a submodule),
is still maintained. DIRGE's changes to Felucca's own files are kept small and marked `DIRGE`.

## Credits and licence

GPL-3.0-only, as Felucca: [LICENSE](LICENSE), with the credits and third-party parts in [LICENSING.md](LICENSING.md)
and [CREDITS.md](CREDITS.md). Felucca and its engines are by Leo Kuroshita, Hügelton Instruments; DIRGE is not made
or endorsed by Hügelton Instruments. "M-VAVE" and "FM-1" are trademarks of their respective owners; DIRGE is not
affiliated with or endorsed by them. Like Felucca, DIRGE never enables the hardware's Bluetooth / Wi-Fi radio.
