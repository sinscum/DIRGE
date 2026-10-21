# FM-1 Dark Firmware Project

Custom firmware for the M-VAVE FM-1, aimed at **ambient, dungeon synth, noise and darksynth**.
The goal is a fixed-point DSP "sound foundation" first, then an instrument built on it.

## The niche

Existing dark firmwares lean medieval/ambient:
- **GHOULBOX** (github.com/jasonpersinger/ghoulbox-fm1-dungeon-synth): a Felucca fork with hall reverb (borrows the delay buffer), tape/crush, a hurdy-gurdy engine, CC0 samples, and host-side audio tests.
- **GR!MM** (github.com/GRiMMBIENT/grimm): built on the Dinghy template with Felucca engines. It is a 4-track drone machine with reverb up to ~32 s plus infinite, granular freeze, and tape.

Nobody covers **noise and darksynth**: aggressive mono basses, chaos and feedback oscillators, distortion chains, gritty arps. That is our gap.

## Platform lineage

Felucca (hugelton) → SLOOP → Dinghy (zvenson) → **foma** (konsumer, Docker + browser emulator).

- Start on **foma**: github.com/konsumer/foma. Run `./fm1 dev` to get the emulator at localhost:8000 with hot reload.
- Do NOT build on M-VAVE's `fm1_open`. Its sound engine is a closed binary SDK and its build is Windows-only.

## Chip rules (JieLi AC791N): the build enforces these

- No FPU. **No float or double.** Use Q15 levels and 32-bit phase accumulators.
- No 64-bit division and no libc (`malloc`, `printf`, `sin`). 64-bit multiplication is fine.
- Division by zero does not trap on hardware; it silently gives garbage. Guard every divisor.
- `audio()` gets 256 frames at 44.1 kHz, about 5.8 ms. No screen, flash or blocking work inside it.
- Share data between `loop()` and `audio()` through `volatile` variables, with one writer each.
- RAM is about 96 KiB for variables; mark big buffers `POOL` for roughly 200 KiB more. `const` tables live in flash.
- Memory budget: 1 s of 16-bit mono audio is about 88 KB, so the pooled memory holds only ~2 s of total delay line. Long reverbs need FDN tricks; long delays need downsampled or 8-bit storage.
- Draw only what changed (a full screen send takes ~80 ms). Save settings only while silent.
- Recovery: hold OCT− at power-on for USB rescue. Back up the full flash (M-Flasher or MvaveFM1Unbricker) before the first install.

## Layout and tests

- `src/main.c`: the firmware. `src/dsp/dsp.h`: the DSP library (Felucca-Engines' `dsp.c`, vendored unchanged in `lib/felucca-engines/`, plus DIRGE's own modules such as `noise.h`). Only `.c` files under `src/` are compiled, so libraries live in `lib/` and are included as one translation unit.
- `sh tests/run.sh` runs everything on the host without Docker: the vendored engines' own goldens, a table drift check, then the DSP blocks (`tests/dsp_test.c`) and the firmware in a scripted pretend FM-1 (`tests/fm1_host.c`) under ASan/UBSan, with health checks and golden hashes in `tests/golden.txt`. WAVs land in `build/tests/`. CI runs it on every push.
- A golden hash change is only fine when the sound was meant to change: then `UPDATE=1 sh tests/run.sh` and say so in the commit.
- Work goes straight to `main` (no feature branches or PRs).

## Roadmap

0. **Play first.** Try the GHOULBOX and GR!MM browser demos and Jangada. Write down what is missing.
1. **Toolchain.** Set up foma in Docker and write a fixed-point detuned drone in `audio()`.
2. **DSP foundation library** (`src/dsp/`), each module with a host WAV render and a golden-hash regression test:
   - Oscillators: band-limited saw and pulse (PolyBLEP), wavetable, feedback/chaos.
   - Noise: white, pink, brown, crackle, LFSR.
   - Filters: SVF and ladder, plus a drive/fold/bitcrush chain.
   - Modulation: long envelopes, slow random LFO, sample-and-hold.
   - Delay line in POOL, compact FDN reverb, tape wow/flutter/saturation.
3. **Decide fork vs scratch.** Either stay on foma, or port the best engines into a Felucca or GR!MM fork to get its sequencer, presets and editor.
4. **Instrument design:** drone layer, darksynth mono bass, noise/percussion voice, and a mangling FX bus. Enable the `midi_clock` option so it slaves to an Elektron Digitakt II and Digitone II.
5. **Release:** pick a unique identity in `firmware.json`, publish via GitHub Actions Pages, test on a real FM-1, then submit to thefirmwarehub.com. The project must stay GPL-3.0.

## References

- Hub: https://thefirmwarehub.com/ · Tools: https://thefirmwarehub.com/tools/
- Felucca build guide: https://github.com/hugelton/Felucca/blob/main/BUILDING.md
- Dinghy docs (`API.md`, `RULES.md`): https://github.com/zvenson/dinghy
- Emulator: https://github.com/simonjohansson/fm1-emulator
