#!/bin/sh
# Portable text snapshot & glyph metric tests; no Windows SDK, SDL, build/gen.
set -eu
cd "$(dirname "$0")"
out="$(mktemp /tmp/hd-text-test.XXXXXX)"
trap 'rm -f "$out"' EXIT HUP INT TERM
for test in hd_text_wire_test hd_text_metrics_test; do
  cc -std=c11 -Wall -Wextra -Werror -Wno-unused-function -O1 -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer "$test.c" -lm -o "$out"
  "$out"
done
