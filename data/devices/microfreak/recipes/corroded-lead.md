---
id: microfreak-corroded-lead
name: Corroded Lead
device: microfreak
category: lead
key_agnostic: false
params:
  - { section: Osc, control: Type, value: "Waveshaper" }
  - { section: Osc, control: Wave, value: "34" }
  - { section: Osc, control: Timbre, value: "79" }
  - { section: Osc, control: Shape, value: "58" }
  - { section: Filter, control: Type, value: "Band pass" }
  - { section: Filter, control: Cutoff, value: "61" }
  - { section: Filter, control: Resonance, value: "72" }
  - { section: Envelope, control: Attack, value: "4" }
  - { section: Envelope, control: Decay, value: "52" }
  - { section: Envelope, control: Sustain, value: "40" }
  - { section: Cycling Env, control: Rise, value: "18" }
  - { section: Cycling Env, control: Fall, value: "36" }
  - { section: Cycling Env, control: Amount, value: "54" }
  - { section: Matrix, control: "Cyc Env → Timbre", value: "+40" }
  - { section: Matrix, control: "Key/Arp → Cutoff", value: "+15" }
  - { section: Voice, control: Para Mode, value: "Off" }
  - { section: Voice, control: Glide, value: "26" }
depends_on:
  clock: external
  fx_chain: ["tape delay: dotted eighth, 35% feedback, filtered repeats", "light drive before the delay"]
audio_ref: ""
---

Mono voice, glide on, meant for one line at a time in the upper register. The
band-pass with high resonance is what makes it read as metal rather than as a
saw lead — leave the resonance where it is even when it feels like too much.

Not key agnostic: the resonant peak sits where it does on purpose. Transpose the
part more than a fourth and re-tune cutoff by ear.
