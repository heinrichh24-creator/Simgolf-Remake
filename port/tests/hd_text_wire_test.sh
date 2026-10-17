#!/bin/sh
# Runs without Windows SDK, SDL, or translated build/gen.
set -eu
cd "$(dirname "$0")"
out="$(mktemp /tmp/hd-text-wire-test.XXXXXX)"
trap 'rm -f "$out"' EXIT HUP INT TERM
cc -std=c11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined \
  -fno-omit-frame-pointer hd_text_wire_test.c -lm -o "$out"
"$out"
