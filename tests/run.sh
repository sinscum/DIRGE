#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# DIRGE's host tests, on any computer with a C compiler and Python 3 (no Docker, no FM-1):
#   1. the vendored Felucca-Engines, against their own golden renders (lib/felucca-engines/tests/run.sh)
#   2. src/dsp/fe_tables.h is what the vendored generator makes (no hand edits, no drift)
#   3. the DSP library (tests/dsp_test.c) and the firmware itself (tests/fm1_host.c + src/*.c), with address and
#      undefined-behaviour checking: their health checks, then their renders against tests/golden.txt
#   sh tests/run.sh               the renders as WAVs in build/tests/ to listen to
#   UPDATE=1 sh tests/run.sh      rewrite tests/golden.txt: only for a reviewed, intended change of the sound
set -e
cd "$(dirname "$0")/.."
OUT=build/tests
mkdir -p "$OUT"
CC=${CC:-cc}

echo "== Felucca-Engines (vendored)"
(cd lib/felucca-engines && UPDATE=0 sh tests/run.sh) | tail -1
rm -rf lib/felucca-engines/build

echo "== tables"
python3 lib/felucca-engines/tools/gen_engine_tables.py "$OUT/fe_tables.h"
cmp -s "$OUT/fe_tables.h" src/dsp/fe_tables.h || {
  echo "FAIL: src/dsp/fe_tables.h differs from what lib/felucca-engines/tools/gen_engine_tables.py makes"
  exit 1
}
echo "ok"

# -fno-sanitize=shift: the fixed-point code shifts negative values left on purpose (as Felucca's stress test)
FLAGS="-O1 -g -std=gnu11 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -fsanitize=address,undefined \
  -fno-sanitize=shift -fno-sanitize-recover=undefined -fno-omit-frame-pointer"
echo "== build"
# shellcheck disable=SC2086
$CC $FLAGS -o "$OUT/dsp_test" tests/dsp_test.c -lm
# shellcheck disable=SC2086
$CC $FLAGS -Iplatform -Iplatform/src -Iplatform/gen -Wno-sign-compare -o "$OUT/fm1_host" tests/fm1_host.c $(find src -name '*.c' | sort) -lm

echo "== renders"
status=0
WAV_DIR="$OUT" "$OUT/dsp_test" > "$OUT/renders.txt" || status=1
WAV_DIR="$OUT" "$OUT/fm1_host" >> "$OUT/renders.txt" || status=1
[ $status = 0 ] || {
  echo "FAIL: health checks (above)"
  exit 1
}

if [ "${UPDATE:-0}" = 1 ]; then
  { echo "# DIRGE's golden renders (tests/run.sh): name, FNV-1a 64 of the 16-bit output."
    echo "# Rewritten by UPDATE=1 sh tests/run.sh -- only for an intended change of the sound."
    cat "$OUT/renders.txt"; } > tests/golden.txt
  echo "tests/golden.txt rewritten ($(wc -l < "$OUT/renders.txt" | tr -d ' ') renders)"
  exit 0
fi
if grep -v '^#' tests/golden.txt | diff -u - "$OUT/renders.txt"; then
  echo "ok: $(wc -l < "$OUT/renders.txt" | tr -d ' ') renders as in tests/golden.txt (WAVs in $OUT/)"
else
  echo "FAIL: the renders above differ from tests/golden.txt (an intended change: UPDATE=1 sh tests/run.sh)"
  exit 1
fi
