---
id: model-cycles-tape-sub
name: Tape Sub
device: model-cycles
category: bass
key_agnostic: true
params:
  - { section: Track, control: Machine, value: "Kick" }
  - { section: Track, control: Track, value: "T1" }
  - { section: Synth, control: Pitch, value: "-24" }
  - { section: Synth, control: Decay, value: "112" }
  - { section: Synth, control: Color, value: "18" }
  - { section: Synth, control: Shape, value: "40" }
  - { section: Synth, control: Sweep, value: "0" }
  - { section: Synth, control: Contour, value: "24" }
  - { section: Mix, control: Level, value: "104" }
  - { section: Mix, control: Pan, value: "Centre" }
  - { section: Mix, control: "Delay send", value: "0" }
  - { section: Mix, control: "Reverb send", value: "6" }
  - { section: Sequencer, control: Length, value: "16" }
  - { section: Sequencer, control: "Note length", value: "Gate held" }
depends_on:
  clock: internal
  fx_chain: ["tape saturation, drive 3", "sidechain compressor keyed from the kick track"]
audio_ref: ""
---

Kick machine with sweep at zero and a decay long enough to sustain — it stops
being a transient and becomes a tone. Sweep must stay at 0; any pitch envelope
turns the sub into a tom.

Reverb send stays at 6 or below. Sub into a large hall is how a mix loses its
floor.
