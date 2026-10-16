#!/usr/bin/env bash
# Offline save-preflight regression: no installed game or build/gen required.
# clang (preferred) or gcc on Linux/WSL; real port/src/port/golf/staff.c.
set -euo pipefail
cd "$(dirname "$0")/../.."
cc="${CC:-clang}"
if ! command -v "$cc" >/dev/null 2>&1; then cc=gcc; fi
tmp="$(mktemp)"
trap 'rm -f "$tmp"' EXIT
"$cc" -D_GNU_SOURCE -D__EMSCRIPTEN__ -std=c11 -O1 -g \
  -Wall -Wextra -ffunction-sections -fdata-sections \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Iport/src -include port/src/port/native.h \
  port/tests/grid_map_tag_preflight_test.c \
  port/src/port/golf/staff.c \
  -Wl,--gc-sections -o "$tmp"
"$tmp"
echo "SGMP preflight regression passed"
