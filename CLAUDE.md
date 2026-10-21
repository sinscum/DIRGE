# FM-1 Dark Firmware Project

Custom firmware for the M-VAVE FM-1, aimed at **ambient, dungeon synth, noise and darksynth**.
The goal is a fixed-point DSP "sound foundation" first, then an instrument built on it.

## The niche

Existing dark firmwares lean medieval/ambient:
- **GHOULBOX** (github.com/jasonpersinger/ghoulbox-fm1-dungeon-synth): a Felucca fork with hall reverb (borrows the delay buffer), tape/crush, a hurdy-gurdy engine, CC0 samples, and host-side audio tests.
- **GR!MM** (github.com/GRiMMBIENT/grimm): built on the Dinghy template with Felucca engines. It is a 4-track drone machine with reverb up to ~32 s plus infinite, granular freeze, and tape.

Nobody covers **noise and darksynth**: aggressive mono basses, chaos and feedback oscillators, distortion chains, gritty arps. That is our gap.

## Platform

DIRGE is a **fork of Felucca 1.6.1** (hugelton/Felucca, GPL-3.0), with Felucca's history merged in (remote `felucca`), so its fixes merge in. Lineage: Felucca → SLOOP → Dinghy → foma. DIRGE started on foma and moved to Felucca for its instrument core: 12 voices with MONO / LEGATO / UNISON and glide, the sequencer and song chain, the modulation matrix, MIDI clock in (USB and TRS: needed for the Digitakt II and Digitone II), projects, presets, the web editor and installer. GHOULBOX is also a Felucca fork; GR!MM (Dinghy) has no mono / glide and no clock in.

- Felucca's author has stepped back (occasional maintenance). Felucca-Engines (`firmware/engines/`, a git submodule, MIT) is still maintained.
- Keep DIRGE's changes to Felucca's files small and mark them `DIRGE`; DIRGE's own code goes in its own files (`firmware/src/eng_*.c` engines, `firmware/src/dirge/`).
- Do NOT build on M-VAVE's `fm1_open`. Its sound engine is a closed binary SDK and its build is Windows-only.

## Chip rules (JieLi AC791N): the build enforces these

- No FPU. **No float or double.** Use Q15 levels and 32-bit phase accumulators.
- No 64-bit division and no libc (`malloc`, `printf`, `sin`). 64-bit multiplication is fine.
- Division by zero does not trap on hardware; it silently gives garbage. Guard every divisor.
- The audio ISR renders 256 frames at 44.1 kHz (~5.8 ms), voices in control blocks of `CTL` = 32 samples. No screen, flash or blocking work in it. Felucca's tests count each engine's CPU (`tests/cpu_baseline.txt`, `tests/target_budget.txt`).
- Share data between the main loop and the audio ISR through `volatile` variables, with one writer each.
- Felucca's budgets (`./build.sh` prints them): RAM .data+.bss 61 of 72 KiB, pool 329 of 360 KiB (Felucca's delay, chorus and reverb buffers are there: ~40 KB free), app image 513 of 568 KB (~55 KB free; the CC0 samples are 126 KB of it and can go). `const` tables live in flash.
- Memory budget: 1 s of 16-bit mono audio is about 88 KB. With ~40 KB of pool free, new delay lines must replace or share Felucca's buffers (GHOULBOX's HALL borrows the delay's); long reverbs need FDN tricks, long delays downsampled or 8-bit storage.
- Draw only what changed (a full screen send takes ~80 ms). Save settings only while silent.
- Recovery: hold OCT− at power-on for USB rescue. Back up the full flash (M-Flasher or MvaveFM1Unbricker) before the first install.

## Layout and tests

- `firmware/src/felucca.c` is the one translation unit; `engines.c` includes the engines. An engine is an `engine_t` (`firmware/engines/src/felucca_engine.h`): eight parameters, presets, `note_on`, `render` (adds a block to a mono buffer through `voice_amp`), optional hooks.
- **Engine numbers are stored** (projects, presets, the editor protocol): append new engines after DRONE (14), never reorder. A new engine also goes in `core.h` NENGINES, `engines.c` ENGINES and ENGINE_ORDER, `params.c` N_ENGNAME, `category.c` PRESET_CAT (a letter per preset), `icons.c`, `ui_graph.c`'s short names and `web/editor.html` (ENG, the ENG names, ENGINE_ORDER). Engines 14 on have no favourites row (`favorites.c` FAV_ROWS); the EDIT layer has 16 white keys (visible engines + INIT + CLEAN INIT: 14 + 2 now, full).
- `firmware/src/dirge/`: DIRGE's DSP beside Felucca-Engines' `dsp.c` (included by `engines.c` right after it).
- Build: `AC79_SDK=sdk/ac79 ./build.sh` (toolchain in `~/.jieli/toolchain`, or `JIELI_TOOLCHAIN`; Pillow with libraqm, fontTools). Emulator: `web/emu/build.sh` (Emscripten; CI uses `emscripten/emsdk:3.1.74`).
- Tests: `AC79_SDK=sdk/ac79 sh tests/run_tests.sh` after a build: Felucca's whole suite (every engine x preset against `tests/golden.txt`, health, CPU budgets, UI renders, storage, MIDI, ...) and DIRGE's DSP tests (`sh tests/dirge/run.sh`, `tests/dirge/golden.txt`). A golden change is fine only when the sound was meant to change: `GOLDEN_UPDATE=1` (regress), `BUDGET_UPDATE=1` (CPU), `UPDATE=1` (tests/dirge), and say so in the commit. CI (`.github/workflows/ci.yml`) runs all of it and publishes the site from `main`.
- Work goes straight to `main` (no feature branches or PRs).

## Roadmap

0. **Play first.** Try the GHOULBOX and GR!MM browser demos and Jangada. Write down what is missing.
1. **Toolchain.** Done (on foma, then moved to Felucca): a fixed-point detuned drone.
2. **DSP foundation library** (`src/dsp/`), each module with a host WAV render and a golden-hash regression test:
   - Oscillators: band-limited saw and pulse (PolyBLEP), wavetable, feedback/chaos.
   - Noise: white, pink, brown, crackle, LFSR.
   - Filters: SVF and ladder, plus a drive/fold/bitcrush chain.
   - Modulation: long envelopes, slow random LFO, sample-and-hold.
   - Delay line in POOL, compact FDN reverb, tape wow/flutter/saturation.
3. **Decide fork vs scratch.** Done: DIRGE is a Felucca fork (see Platform); step 1's drone is the DRONE engine.
4. **Instrument design:** drone layer, darksynth mono bass, noise/percussion voice, and a mangling FX bus. Enable the `midi_clock` option so it slaves to an Elektron Digitakt II and Digitone II.
5. **Release:** check the identity (FM-1_6XY, `tools/build.py`) against the hub, publish via GitHub Actions Pages, test on a real FM-1, then submit to thefirmwarehub.com. The project must stay GPL-3.0.

## References

- Hub: https://thefirmwarehub.com/ · Tools: https://thefirmwarehub.com/tools/
- Felucca build guide: https://github.com/hugelton/Felucca/blob/main/BUILDING.md
- Dinghy docs (`API.md`, `RULES.md`): https://github.com/zvenson/dinghy
- Emulator: https://github.com/simonjohansson/fm1-emulator
- Felucca: https://github.com/hugelton/Felucca · Felucca-Engines: https://github.com/hugelton/Felucca-Engines
