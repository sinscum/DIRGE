#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 DIRGE
# DIRGE's DSP tests (tests/dirge/dsp_test.c), on any computer with a C compiler and Python 3: Felucca-Engines' dsp.c
# blocks and DIRGE's own (firmware/src/dirge/) rendered with address and undefined-behaviour checking, their health
# checks, then their hashes against tests/dirge/golden.txt. The DRONE engine itself is in Felucca's regression suite
# (tests/regress.c: every engine x preset), which tests/run_tests.sh runs, as it runs this.
#   sh tests/dirge/run.sh               the renders as WAVs in build/dirge/
#   UPDATE=1 sh tests/dirge/run.sh      rewrite tests/dirge/golden.txt: only for an intended change of the sound
set -e
cd "$(dirname "$0")/../.."
OUT=build/dirge
mkdir -p "$OUT"
python3 firmware/engines/tools/gen_engine_tables.py "$OUT/fe_tables.h"
# -fno-sanitize=shift: the fixed-point code shifts negative values left on purpose (as Felucca's own tests)
${CC:-cc} -O1 -g -std=gnu11 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -fsanitize=address,undefined \
  -fno-sanitize=shift -fno-sanitize-recover=undefined -fno-omit-frame-pointer -I"$OUT" \
  -o "$OUT/dsp_test" tests/dirge/dsp_test.c -lm
WAV_DIR="$OUT" "$OUT/dsp_test" > "$OUT/renders.txt" || { echo "FAIL: health checks (above)"; exit 1; }
if [ "${UPDATE:-0}" = 1 ]; then
  { grep '^#' tests/dirge/golden.txt; cat "$OUT/renders.txt"; } > "$OUT/golden.new" && mv "$OUT/golden.new" tests/dirge/golden.txt
  echo "tests/dirge/golden.txt rewritten ($(wc -l < "$OUT/renders.txt" | tr -d ' ') renders)"
  exit 0
fi
if grep -v '^#' tests/dirge/golden.txt | diff -u - "$OUT/renders.txt"; then
  echo "ok: $(wc -l < "$OUT/renders.txt" | tr -d ' ') DSP renders as in tests/dirge/golden.txt (WAVs in $OUT/)"
else
  echo "FAIL: the renders above differ from tests/dirge/golden.txt (an intended change: UPDATE=1 sh tests/dirge/run.sh)"
  exit 1
fi
