---
id: black-tide
title: Black Tide
key: F
mode: phrygian
bpm: 96
status: sketch
progression:
  - { bar: 1, chord: "Fm", duration: 4 }
  - { bar: 5, chord: "Gb", duration: 4 }
  - { bar: 9, chord: "Db", duration: 2 }
  - { bar: 11, chord: "Cm", duration: 2 }
layers:
  - slot: sub
    device: model-cycles
    recipe: model-cycles-tape-sub
    register: "C1"
    enters_bar: 1
    exits_bar: 64
    notes: "Sidechained to kick"
  - slot: pad
    device: microfreak
    recipe: microfreak-ashen-choir
    register: "F3"
    enters_bar: 9
    exits_bar: 56
suno_overrides:
  reference_artists: []
  production: "analog tape saturation, wide stereo, no vocals"
---

Stalking tempo, bII on bar 5 doing the work. The pad stays out until bar 9 so
the first eight bars are sub and room only.
