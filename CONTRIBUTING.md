# Contributing to DIRGE

Thanks for helping! DIRGE is firmware for the M-VAVE FM-1, a fork of [Felucca](https://github.com/hugelton/Felucca).
Please report DIRGE's behaviour here, not in Felucca's tracker: Felucca's author does not support forks.

| What | Where |
|---|---|
| A bug you can reproduce | [Issues › Bug report](https://github.com/sinscum/DIRGE/issues/new?template=bug_report.yml): one issue per bug |
| A feature idea, a sound or engine request | [Issues › Idea](https://github.com/sinscum/DIRGE/issues/new?template=idea.yml) (no promise it will be added) |
| How to use it | the [manual](MANUAL.md) first, then an issue with "Question:" in the title |
| A bug in the ANALOG, LOFI, TRIO, WHEEL or NOISE engines that Felucca has too | [Felucca-Engines](https://github.com/hugelton/Felucca-Engines/issues) |

## Reporting a bug

Please include the DIRGE version (hold HOME > SYSTEM > ABOUT), your computer's OS and browser if the editor,
installer or demo is involved, the steps to reproduce it, what you expected and what happened. A photo, a short video
or a recording helps. If it happens in the [browser demo](https://sinscum.github.io/DIRGE/) too, say so: that makes
it much easier to find.

## Pull requests

Pull requests are welcome, including AI-assisted ones, with the same bar for all:

- keep it small and focused on one thing;
- make sure you understand the change and can explain it;
- follow the chip's rules in [CLAUDE.md](CLAUDE.md): no float or double, no 64-bit division, no libc, every divisor
  guarded, nothing blocking in the audio interrupt;
- keep DIRGE's changes to Felucca's own files small and marked `DIRGE`; new DIRGE code goes in its own files
  (`firmware/src/eng_*.c` engines, `firmware/src/dirge/`);
- never renumber engines or change a stored format: engines are appended after DRONE, and stored values are
  append-only (CLAUDE.md lists every place a new engine goes);
- a DIRGE feature that is off must leave the sound bit for bit as Felucca's;
- run `AC79_SDK=sdk/ac79 sh tests/run_tests.sh` (see [BUILDING.md](BUILDING.md)): every host test passes. Update a
  golden (`GOLDEN_UPDATE=1`, `BUDGET_UPDATE=1`, `UPDATE=1` for `tests/dirge/`) only when the sound was meant to
  change, and say so in the commit;
- test it on a real FM-1 when it touches the hardware, timing or storage;
- new files carry the `SPDX-License-Identifier: GPL-3.0-only` header;
- bring in only code whose licence allows it (GPL-3.0-compatible), and credit it in [CREDITS.md](CREDITS.md) and
  [LICENSING.md](LICENSING.md). Code ported from another FM-1 firmware (Skiff, GR!MM, GHOULBOX, ...) names its
  origin in a comment and in the commit.

By contributing you agree that your contribution is licensed under GPL-3.0-only.

Changes to the MIT engines in `firmware/engines/` (a git submodule) belong in
[Felucca-Engines](https://github.com/hugelton/Felucca-Engines), as pull requests to that repository.
