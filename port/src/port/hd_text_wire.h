// Native text handoff wire-format validator. Portable C: no Win32/SDL headers.
// Each run: 13 metadata floats followed by 3 floats per glyph (code,x,baseline).
// Runs arrive newest-first. A consumer must replay them oldest-first.
#ifndef SIMGOLF_HD_TEXT_WIRE_H
#define SIMGOLF_HD_TEXT_WIRE_H
#include <math.h>

#define HD_TEXT_WIRE_CAP 32768
#define HD_TEXT_WIRE_META 13
#define HD_TEXT_WIRE_GLYPH 3

static inline int hd_text_wire_validate(const float *data, int count,
                                         int *run_count, int *glyph_count) {
  int at = 0, nr = 0, ng = 0;
  if (run_count) *run_count = 0;
  if (glyph_count) *glyph_count = 0;
  if (count < 0 || count > HD_TEXT_WIRE_CAP || (count && !data)) return 0;
  while (at < count) {
    if (count - at < HD_TEXT_WIRE_META) return 0;
    for (int j = 0; j < HD_TEXT_WIRE_META; j++)
      if (!isfinite(data[at + j])) return 0;
    float n = data[at + 12];
    if (!(n >= 0 && n <= (float)((count - at - HD_TEXT_WIRE_META) / HD_TEXT_WIRE_GLYPH))) return 0;
    int glyphs = (int)n;
    if ((float)glyphs != n) return 0;
    for (int j = 0; j < 12; j++)
      if (data[at + j] < -1000000.0f || data[at + j] > 1000000.0f) return 0;
    for (int j = 0; j < glyphs * HD_TEXT_WIRE_GLYPH; j++)
      if (!isfinite(data[at + HD_TEXT_WIRE_META + j]) ||
          data[at + HD_TEXT_WIRE_META + j] < -1000000.0f ||
          data[at + HD_TEXT_WIRE_META + j] > 1000000.0f) return 0;
    at += HD_TEXT_WIRE_META + glyphs * HD_TEXT_WIRE_GLYPH;
    nr++;
    ng += glyphs;
  }
  if (run_count) *run_count = nr;
  if (glyph_count) *glyph_count = ng;
  return 1;
}
#endif
