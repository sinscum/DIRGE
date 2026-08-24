# DIRGE

*Dark Instrument Reference & Generative Environment* — a self-hosted reference
manual and composition workspace for a darksynth / horrorsynth hardware studio.

**Phase 1 (Rack) is built.** Filesystem loader, zod schemas, device index,
device pages, recipe pages, search, and the print stylesheet. Forge — the
progression builder, arrangement planner, and Suno generator — is phases 2–4.

## Run it

```sh
npm install
npm run dev          # http://localhost:5173
```

Production:

```sh
npm run build
node build/index.js  # PORT and DIRGE_DATA_DIR are read from the environment
```

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

`key`, `mode`, `bpm`, `progression`, and `layers`. A layer's `recipe` field is
the back-reference that makes a patch a referenceable object rather than a note.

### Validation

Every file is validated with zod on read. A failure never crashes the page and
never drops content silently — the page renders with an error panel naming the
file and each failing field.

## Print

Device pages and recipe pages print as 5.5×8.5in half-letter, black on white,
no chrome, parameter tables intact. Use the **Print** button on either page.
