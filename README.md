# DIRGE

*Dark Instrument Reference & Generative Environment* — a self-hosted reference
manual and composition workspace for a darksynth / horrorsynth hardware studio.

**Phases 1–2 are built.** Rack: filesystem loader, zod schemas, device index,
device pages, recipe pages, search, print stylesheet. Forge: the progression
builder — mode-aware chord palette, chromatic mediants, bar timeline, drone
lane, staff view, and a copyable note grid. The arrangement planner and Suno
generator are phases 3–4.

## Run it

```sh
npm install
npm run dev          # http://localhost:5173
```

Production:

```sh
npm run build
ORIGIN=https://dirge.example node build/index.js
```

Environment:

| variable | what it does |
| --- | --- |
| `PORT` | port to listen on (default 3000) |
| `DIRGE_DATA_DIR` | content directory (default `./data`) |
| `ORIGIN` | the URL the app is served at — **required** |

`ORIGIN` is not optional once anything writes: SvelteKit rejects form posts
whose `Origin` header doesn't match the app's own origin, and behind a reverse
proxy it cannot work that out on its own. Without it, saving a progression
fails with *Cross-site POST form submissions are forbidden*.

`npm run check` runs `svelte-check` over the whole project.

## Data

The filesystem is the database. Nothing is baked at build time — every request
reads the files, so editing a `.md` in an editor shows up on refresh.

```
data/
  devices/
    microfreak/
      device.md          # frontmatter + the five fixed sections
      recipes/
        ashen-choir.md   # one patch recipe per file
    model-cycles/
  songs/
    black-tide.md
  templates/
    arrangements.yaml
```

Point `DIRGE_DATA_DIR` at another directory to use it instead of `./data`.

### device.md

Frontmatter carries `id`, `name`, `type` (synth · drum · fx · controller ·
recorder · utility), and the rig facts. The body uses `##` headings in a fixed
order — **Vitals, Shortcuts, Signal chain, Troubleshooting, Resources**. A
missing section renders as a stub with a hint instead of vanishing.

Inside Troubleshooting, each symptom is a `###` heading followed by cause then
fix. Those headings are what search matches on.

### recipe.md

Lives in the device's `recipes/` folder. `id` is globally unique and prefixed
with the device id (`microfreak-ashen-choir`) — that id is what a song layer
points at, and what the recipe page's **Used in** table reverses.

`params` is a flat list of `{ section, control, value }`; the recipe page groups
it by `section` in file order.

### song.md

`key`, `mode`, `bpm`, `progression`, `drone`, and `layers`. A layer's `recipe`
field is the back-reference that makes a patch a referenceable object rather
than a note.

`drone` is the pedal tone under the progression: `{ note, preset }`, where
preset is `tonic`, `tritone`, or `custom`. It is optional — songs written before
it existed still validate.

### Validation

Every file is validated with zod on read. A failure never crashes the page and
never drops content silently — the page renders with an error panel naming the
file and each failing field.

## Forge

`/forge` lists the songs in `data/songs` and starts new ones. Opening one gives
the progression builder:

- **Palette** — the diatonic triads of the chosen mode. Aeolian, Phrygian,
  Harmonic minor, Locrian and Dorian sit up front; the rest are behind *more*.
  Diminished and augmented chords are marked as passing colour, and the ♭II is
  emphasised wherever the mode has one.
- **Wrong in a good way** — the ♭VI, ♭III and III majors measured from whatever
  chord you last placed. Chromatic mediant roots are respelled to something
  playable, so a move off A♭ offers E, not F♭.
- **Bars** — one block per chord, sized by its length, reorderable, with the
  drone lane beneath.
- **Staff** — root-position triads on a treble staff, every accidental spelled
  out, the pedal drawn as a held line underneath.
- **Note grid** — the same thing as text, for punching into a hardware
  sequencer by hand.

Saving writes the song's markdown file. Unknown frontmatter keys you added by
hand are preserved, but the file is re-serialised: YAML comments are lost and
schema defaults are filled in. Nothing is written unless the result passes
`songSchema` — a rejected save says which field failed and leaves the file alone.

## Print

Device pages and recipe pages print as 5.5×8.5in half-letter, black on white,
no chrome, parameter tables intact. Use the **Print** button on either page.
