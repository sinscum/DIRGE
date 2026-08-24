---
id: model-cycles
name: Elektron Model:Cycles
type: drum
firmware: "1.10"
polyphony: 6
engines: [kick, snare, metal, perc, tone, chord]
io: { audio_out: "1/4in stereo L/R", midi: "3.5mm TRS in/out/thru", clock: "MIDI clock master", usb: "USB audio + MIDI" }
midi_channel: 1
role: "Clock master, sub, perc, FM metal textures"
manual_url: "https://www.elektron.se/support/model-cycles"
tags: [hardware, fm, groovebox, clock-master]
---

## Vitals

Six FM tracks, one machine per track, six voices total. Every track is the same
six parameters wearing different names, which is why it is fast: PITCH, DECAY,
COLOR, SHAPE, SWEEP, CONTOUR, plus level, pan and the two sends.

- Machines: **Kick, Snare, Metal, Perc, Tone, Chord.** Kick with a long decay
  and no sweep is a usable sub. Metal is where the horror lives — inharmonic FM
  ratios that never resolve.
- Delay and reverb are send effects, shared across all six tracks. There is no
  per-track reverb, so one track's tail decides the mood of everything.
- Sequencer: 16 steps per pattern per track, parameter locks on any step,
  per-track length and speed for polymetric parts.
- No sampling, no audio in. Whatever it plays, it synthesizes.
- Project auto-saves. Patterns do not — save before switching if you changed one.

## Shortcuts

- **Hold FUNC while powering on** — early startup menu: test mode, OS upgrade,
  factory reset. Factory reset wipes every pattern; back up over USB first.
- **FUNC + REC / STOP / PLAY** — copy, paste, clear. Works on whatever is in
  focus: a step, a track, or the whole pattern.
- **FUNC + a knob** — fine adjust on that parameter.
- **Hold a step + turn a knob** — parameter lock that value to that step. The
  single most useful thing this machine does.
- **FUNC + PAGE** — change pattern length and per-track speed for polymeter.
- **Hold PTN + a pad** — jump to that pattern; the change lands at the end of
  the current loop, not instantly.
- **Hold a track button** — solo/mute state and track focus without stopping.

## Signal chain

**This is the clock master for the whole rig.** MIDI out (3.5mm TRS → DIN
adapter) feeds the MicroFreak clock in and the interface. Nothing sends clock to
it.

Stereo out L/R into mixer channels 1–2. The reverb send stays low here because
the room comes from the outboard reverb on the master bus — doubling them turns
the low end to mud.

Kick track is sidechain source: its trigger drives the compressor on the sub
bus, so the sub ducks even when the kick itself is muted in the mix.

## Troubleshooting

### MicroFreak does not follow when transport starts

**Cause.** Clock is enabled in the Cycles but transport send is off, or the TRS
adapter is a Type B in a Type A jack.

**Fix.** Settings → MIDI Config → Port Config: clock send **on**, transport send
**on**. Confirm the adapter type; the Cycles is TRS Type A. Restart transport
after changing either.

### Pattern changes revert after switching back

**Cause.** Pattern edits are not written to the project until saved. Switching
patterns discards unsaved changes to the one you left.

**Fix.** Save the pattern before you switch. Treat live pattern tweaks as
throwaway unless you save deliberately.

### Low end disappears when the kick and sub play together

**Cause.** Two FM tracks with fundamentals a few Hz apart phase-cancel — the
Kick machine sub and the Tone machine bass fighting for the same 40–60Hz.

**Fix.** Move the sub down an octave or the kick up a semitone, and shorten the
kick decay so the sustain belongs to one track only. Sidechain is a fix for
level, not for cancellation.

### Distorted output even with track levels down

**Cause.** Master delay and reverb sends still sum after the track fader on the
way into the master, and the master level itself is post-effects.

**Fix.** Pull the sends before the fader chase. If it persists, drop master
volume in Settings and take the gain back at the mixer.

## Resources

- [Model:Cycles support and manual](https://www.elektron.se/support/model-cycles)
- Elektron Transfer — OS updates and project backup over USB.
- Overbridge does not support this box; record audio out or USB audio directly.
