// Regression tests for the 4096-byte save/course-picker text buffer.
// Compile with clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined
// and -Iport/src port/tests/file_list_text_bounds_test.c -o /tmp/file_list_text_bounds_test
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "port/golf/re_bounds.h"

enum { CAPACITY = 4096 };

typedef struct {
  uint8_t before[16];
  char text[CAPACITY];
  uint8_t after[16];
} Guarded;

static void guards(const Guarded *g) {
  for (unsigned i = 0; i < 16; ++i) {
    assert(g->before[i] == 0xa5);
    assert(g->after[i] == 0x5a);
  }
}

int main(void) {
  Guarded g;
  const char *append = "Long-SimGolf-course-name.sve";
  for (int used = 0; used < CAPACITY; ++used) {
    memset(&g, 0, sizeof g);
    memset(g.before, 0xa5, sizeof g.before);
    memset(g.after, 0x5a, sizeof g.after);
    memset(g.text, 'A', (size_t)used);
    g.text[used] = 0;
    unsigned available = (unsigned)(CAPACITY - 1 - used);
    unsigned want = (unsigned)strlen(append);
    unsigned got = available < want ? available : want;
    int complete = sg_re_bounded_append(g.text, CAPACITY, append);
    assert(complete == (got == want));
    assert(memcmp(g.text + used, append, got) == 0);
    assert(g.text[used + got] == 0);
    assert(g.text[CAPACITY - 1] == 0 || used + got < CAPACITY - 1);
    guards(&g);
  }

  // Unterminated text must never result in an out-of-bounds scan or append.
  memset(g.text, 'X', sizeof g.text);
  assert(sg_re_bounded_append(g.text, CAPACITY, append) == 0);
  assert(g.text[CAPACITY - 1] == 0);
  guards(&g);
  // Subsequent append attempts stay inside the repaired boundary.
  assert(sg_re_bounded_append(g.text, CAPACITY, append) == 0);
  guards(&g);

  // Small-capacity and invalid-input behavior.
  char tiny[2] = {'\0', 'Z'};
  assert(sg_re_bounded_append(tiny, 1, "xyz") == 0);
  assert(tiny[0] == '\0' && tiny[1] == 'Z');
  assert(sg_re_bounded_append(tiny, 2, "") == 1);
  assert(sg_re_bounded_append(tiny, 0, "a") == 0);
  assert(sg_re_bounded_append(0, 2, "a") == 0);
  assert(sg_re_bounded_append(tiny, 2, 0) == 0);

  // Repeated concatenations emulate filenames and separators in a listing.
  memset(g.text, 0, sizeof g.text);
  for (int i = 0; i < 1000; ++i) {
    sg_re_bounded_append(g.text, CAPACITY, "SaveName.sve");
    sg_re_bounded_append(g.text, CAPACITY, "\n");
  }
  assert(g.text[CAPACITY - 1] == 0);
  guards(&g);
  return 0;
}
