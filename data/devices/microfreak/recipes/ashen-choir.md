---
id: microfreak-ashen-choir
name: Ashen Choir
device: microfreak
category: pad
key_agnostic: true
params:
  - { section: Osc, control: Type, value: "Harmonic OSC" }
  - { section: Osc, control: Wave, value: "62" }
  - { section: Osc, control: Timbre, value: "44" }
  - { section: Osc, control: Shape, value: "71" }
  - { section: Filter, control: Type, value: "Low pass" }
  - { section: Filter, control: Cutoff, value: "38" }
  - { section: Filter, control: Resonance, value: "26" }
  - { section: Envelope, control: Attack, value: "48" }
  - { section: Envelope, control: Decay, value: "70" }
  - { section: Envelope, control: Sustain, value: "82" }
  - { section: Cycling Env, control: Rise, value: "88" }
  - { section: Cycling Env, control: Fall, value: "74" }
  - { section: Cycling Env, control: Amount, value: "31" }
  - { section: Matrix, control: "Cyc Env → Cutoff", value: "+18" }
  - { section: Matrix, control: "Press → Timbre", value: "+22" }
  - { section: Voice, control: Para Mode, value: "On" }
  - { section: Voice, control: Glide, value: "12" }
depends_on:
  clock: external
  fx_chain: ["reverb: large hall, 60% wet, pre-delay 40ms", "high-pass at 180Hz after the reverb"]
audio_ref: ""
---

Four voices through one filter, opening slowly. Play it as held triads and let
the cycling envelope do the moving — hands still, the patch breathes on its own.

The high-pass after the reverb is not optional: without it the pad and the sub
occupy the same 100Hz and the whole low end goes grey.
