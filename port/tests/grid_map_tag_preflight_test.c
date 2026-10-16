// Run production staff_preflight() on synthetic legacy and SGMP-tagged saves.
// This POSIX regression needs no game assets or generated C.
#define _GNU_SOURCE 1
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "port/grid.h"
#include "port/staff.h"

// Representative original tile/corner grids for serializer field sizing.
const GridArray grid_arrays[] = {{0x5722e8u, 1, 0}, {0x5a4998u, 1, 1}};
const int grid_count = sizeof(grid_arrays) / sizeof(grid_arrays[0]);

static char filename[] = "/tmp/simgolf-mapcheck-XXXXXX";
const char *crt_real_path(unsigned fd) { (void)fd; return filename; }
int crt_tell(unsigned fd) { (void)fd; return 0; }

static int run(int w, int h, int version, int course, int tagged) {
  FILE *f = fopen(filename, "wb");
  assert(f);
  if (tagged) {
    assert(fwrite("SGMP", 1, 4, f) == 4);
    uint32_t header[3] = {(uint32_t)version, (uint32_t)w, (uint32_t)h};
    assert(fwrite(header, sizeof header, 1, f) == 1);
  }
  // Adequate body length for both file variants; all other fields zero.
  unsigned char zero[4096] = {0};
  for (int i = 0; i < 600; ++i) assert(fwrite(zero, sizeof zero, 1, f) == 1);
  assert(fclose(f) == 0);
  return staff_preflight(1, course, 0);
}

int main(void) {
  int fd = mkstemp(filename);
  assert(fd >= 0);
  close(fd);
  int bad = 0;
  for (int course = 0; course <= 1; ++course) {
    // Untagged original games/courses must remain accepted.
    bad += run(50, 50, 1, course, 0) != 1;
    const int valid[] = {50, 70, 100, 158, 160};
    for (unsigned i = 0; i < sizeof valid / sizeof *valid; ++i)
      bad += run(valid[i], valid[i], 1, course, 1) != 1;
    bad += run(51, 51, 1, course, 1) != 0;
    bad += run(50, 160, 1, course, 1) != 0;
    bad += run(160, 50, 1, course, 1) != 0;
    bad += run(161, 161, 1, course, 1) != 0;
    bad += run(70, 70, 2, course, 1) != 0;
  }
  assert(unlink(filename) == 0);
  if (bad) fprintf(stderr, "grid map tag regression failures: %d\n", bad);
  return !!bad;
}
