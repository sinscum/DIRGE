---
id: microfreak
name: Arturia MicroFreak
type: synth
firmware: "6.0"
polyphony: 4
engines: [wavetable, karplus, harmonic-osc, modal, vocoder]
io: { audio_out: "1/4in mono", midi: "DIN + USB", clock: "DIN sync" }
midi_channel: 3
role: "Lead, plucked textures, evolving pads"
manual_url: "https://www.arturia.com/products/hardware-synths/microfreak/resources"
tags: [hardware, paraphonic, digital-osc, analog-filter]
---

## Vitals

Digital oscillator into an analog Steiner-Parker filter. Four voices, but
**paraphonic** — they share one filter and one envelope, so a chord is one
shape opening and closing, not four independent notes. That limitation is the
sound: pads bloom as a single mass instead of a stack.

- Oscillator is one engine at a time. Type picks the engine, then Wave, Timbre
  and Shape mean something different in each one.
- Steiner-Parker filter self-oscillates past about 90 resonance. Useful as a
  sine drone when the oscillator is doing something percussive.
- Cycling envelope is a second modulation source that free-runs — the closest
  thing on this box to an LFO with a shape you can bend.
- Mod matrix is 5×7 with three assignable destinations. Assign slots are the
  only way to reach anything not on the front panel.
- Keyboard is a capacitive touch plate with per-key aftertouch. No moving parts,
  no velocity from a key strike — velocity comes from finger area.

## Shortcuts

- **Panel** — jumps from the stored preset values to the current physical knob
  positions. Press it before editing or the first knob you touch will jump.
- **Hold Shift** — the utility functions in blue appear on the OLED. Turn the
  preset encoder to move through them, press to enter.
- **Utility → MIDI → User Ch** — MIDI channel. Set to **3** here; the sequencer
  sends on the same channel.
- **Utility → Preset → Para Mode** — paraphony on or off. Off gives a mono voice
  with the full envelope per note, which is what the lead patches want.
- **Utility → CV/Clock** — clock rate. Must match whatever is master: 1 pulse,
  24ppq or 48ppq.
- **Utility → Misc → Calibration** — keyboard and knob calibration. Run it when
  the touch plate starts dropping notes.
- **Dice** randomizes the sequence, **Spice** bends how far the randomization
  goes. Dice first, then walk Spice up slowly — past halfway it stops being the
  same part.
- **Hold a step + turn a knob** in the sequencer records that knob as automation
  on that step. Four automation tracks per sequence.

## Signal chain

MicroFreak out (mono, 1/4in) → Strymon reverb → mixer channel 3, panned centre
for pads and hard left when it's a counter-arp against the Model:Cycles.

Clock comes **in** from the Model:Cycles — the Cycles is master for the whole
rig. MicroFreak clock in via the 3.5mm sync jack, set to 24ppq. MIDI DIN out
goes to the interface for recording note data; nothing downstream listens to it.

It is a mono source. Anything that needs to sit wide gets width from the reverb
or from a double-tracked take, never from the synth.

## Troubleshooting

### Notes drop out or retrigger while holding the touch plate

**Cause.** The capacitive keyboard reads finger contact area. Dry skin, cold
hands, or a drifted calibration all read as a lighter touch.

**Fix.** Utility → Misc → Calibration, hands off the plate while it runs. If it
still drops, touch with more of the fingertip flat rather than the nail edge.

### Chords sound like one note choking the others

**Cause.** Paraphonic mode with a short decay and low sustain — one shared
envelope is retriggering on every new note in the chord.

**Fix.** Raise sustain, lengthen attack past about 20, and play the chord as one
gesture. If you need independent decay per note, that is a job for the
Model:Cycles Chord machine, not this box.

### Arpeggiator drifts against the rest of the rig

**Cause.** Clock rate mismatch — the Cycles is sending 24ppq and the MicroFreak
is set to 1 pulse per step, or the sync cable is in the out jack.

**Fix.** Utility → CV/Clock, set the rate to match the master, and confirm the
cable is in **Clock In**. Stop and restart transport on the master so both
devices agree on the downbeat.

### No sound at all, screen and LEDs alive

**Cause.** Usually the amp envelope: with Amp Mod off, the cycling envelope is
holding the VCA closed. Occasionally the filter cutoff is parked at zero after a
preset change.

**Fix.** Press Panel to reveal true knob positions, sweep cutoff up, and toggle
Amp Mod. If it is still silent, the oscillator engine may be a vocoder engine
with no input — switch Type to Basic Waves to confirm the voice is alive.

## Resources

- [MicroFreak resources and manual](https://www.arturia.com/products/hardware-synths/microfreak/resources)
- MIDI Control Center — firmware updates and preset backup. Back up before every
  firmware bump; presets have been lost across major versions.
- Local: patch sheets printed from this rack live in the folder under the desk.
