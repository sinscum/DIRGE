# Vendored libraries

Copied in unchanged, so the build needs no submodules. Update one by copying the upstream tree over it, then run
`sh tests/run.sh`.

| Directory | Upstream | Commit | Licence |
| --- | --- | --- | --- |
| `felucca-engines/` | [hugelton/Felucca-Engines](https://github.com/hugelton/Felucca-Engines) (Leo Kuroshita, Hügelton Instruments) | `c46e008` | MIT |

They live outside `src/` because foma compiles every `.c` under `src/` on its own; these are included as one
translation unit instead (`src/dsp/dsp.h`).
