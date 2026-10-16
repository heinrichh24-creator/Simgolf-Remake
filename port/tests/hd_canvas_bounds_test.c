// Boundary regression for HD game-canvas dimensions.
// Compile: clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined
//   -Iport/src port/tests/hd_canvas_bounds_test.c -o /tmp/hd_canvas_bounds_test
#include <assert.h>
#include <stdio.h>
#include "port/hd_canvas_bounds.h"

// This is the existing generic size parser's acceptance policy. Window and
// off-screen render settings deliberately keep using it without the new guard.
static int parse_size(const char *v, int *w, int *h) {
  int a = 0, b = 0;
  if (sscanf(v, "%dx%d", &a, &b) == 2 && a >= 160 && b >= 120 && a <= 16384 && b <= 16384) {
    *w = a;
    *h = b;
    return 1;
  }
  return 0;
}

static int canvas(const char *setting, int *w, int *h) {
  if (!parse_size(setting, w, h) || !hd_canvas_dimensions_supported(*w, *h)) {
    *w = *h = 0;  // same fallback as hd_native.c
    return 0;
  }
  return 1;
}

int main(void) {
  struct {const char *setting; int allowed;} cases[] = {
    {"800x600", 1}, {"4096x4096", 1}, {"1920x1080", 1},
    {"3840x2160", 1}, {"4096x600", 1}, {"800x4096", 1},
    {"160x120", 0}, {"799x600", 0}, {"800x599", 0},
    {"4097x600", 0}, {"800x4097", 0}, {"16384x16384", 0},
    {"16385x16385", 0}, {"-1x1080", 0}, {"invalid", 0}
  };
  for (unsigned i = 0; i < sizeof(cases) / sizeof(*cases); ++i) {
    int w = -1, h = -1;
    int ok = canvas(cases[i].setting, &w, &h);
    assert(ok == cases[i].allowed);
    if (!ok) assert(w == 0 && h == 0);
  }

  // Sweep valid and invalid sizes, including boundaries on both axes.
  for (int w = 160; w <= 4250; w += 7) {
    for (int h = 120; h <= 4250; h += 13) {
      char setting[64];
      snprintf(setting, sizeof setting, "%dx%d", w, h);
      int actual_w = -1, actual_h = -1;
      int ok = canvas(setting, &actual_w, &actual_h);
      int supported = w >= 800 && h >= 600 && w <= 4096 && h <= 4096;
      assert(ok == supported);
      if (ok) assert(actual_w == w && actual_h == h);
      else assert(actual_w == 0 && actual_h == 0);
    }
  }
  return 0;
}
