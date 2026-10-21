# DIRGE

Firmware for the M-VAVE FM-1, made with [foma](https://github.com/konsumer/foma).

Try it in the browser, and install it on the FM-1 from Chrome or Edge: <https://sinscum.github.io/DIRGE/>

## Playing it (v0.1: a detuned drone)

| Control       | Does                                                               |
| ------------- | ------------------------------------------------------------------ |
| keys          | set the drone's root and start it; the same key again fades it out |
| PLAY          | start / stop the drone on the last root                            |
| OCT- / OCT+   | octave                                                             |
| KNOB1 DETUNE  | 0..25 cents between the four saws                                  |
| KNOB2 TONE    | one-pole lowpass, dark to open                                     |
| KNOB3 SUB     | sine an octave under the root                                      |
| KNOB4 DRIFT   | slow random pitch wander per saw                                   |

A MIDI note in (USB or TRS) sets the root and starts it. Octave and knobs are kept in flash, saved once it is silent.

## Develop

Only Docker is needed:

```sh
./fm1 dev          # http://localhost:8000: the emulator, the installer, the rescue; rebuilds when you save
./fm1 build        # build/dirge-VERSION.fwsc
```

The firmware is in `src/`; what it can call is in [platform/fm1.h](platform/fm1.h). Name, version and package
identity: [firmware.json](firmware.json). The rules of the chip (no floats, keep `audio()` short, ...) are in
[foma's README](https://github.com/konsumer/foma#the-rules-of-the-chip).

Every push to `main` publishes the emulator and the installer to GitHub Pages (Settings → Pages → Source: GitHub
Actions); a tag (`v0.2`) also makes a release with the `.fwsc`.

Installing firmware is at your own risk. If the FM-1 does not start, hold OCT- while switching it on (the USB rescue),
then install from the page.

## License

GPL-3.0-only. Credits and third-party parts: [LICENSING.md](LICENSING.md).
