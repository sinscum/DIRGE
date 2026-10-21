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

- **DRONE** (`firmware/src/eng_drone.c`), first in the PRESETS engine list (power-on still plays Felucca's
  sounds; a DIRGE scene comes later): four polyBLEP saws, two at the note and
  two an octave down, pushed apart by DTN and pulled about by DRFT (one slow random walk, each saw following it its
  own way), a sine SUB, BRTH (noise), DRV (tanh) and a rounded resonant low-pass. HOME's knobs: DTN, CUT, SUB, DRFT.
  Presets DIRGE, BLACK MASS, SHIVER, FURNACE, ASH WIND. Up to 4 voices (16 saws).
- **DIRGE's DSP** (`firmware/src/dirge/`), beside Felucca-Engines' `dsp.c`: the rounded SVF low-pass (no DC creeping
  into a long low drone) and white, pink, brown, crackle and shift-register noise.
- **The site**: the emulator in DIRGE's dead-channel room, with a live oscilloscope of its output.

Next: distortion, fold and crush; chaos and feedback oscillators; a mono darksynth bass; slow random modulation; a
long FDN reverb and tape. See [CLAUDE.md](CLAUDE.md) for the plan and the chip's rules.

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
