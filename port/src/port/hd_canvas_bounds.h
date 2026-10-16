#ifndef SIMGOLF_HD_CANVAS_BOUNDS_H
#define SIMGOLF_HD_CANVAS_BOUNDS_H

// Game canvas is distinct from the output/window size. The base game UI
// requires at least 800x600, and the native DIB is limited to 4096 per axis.
static inline int hd_canvas_dimensions_supported(int w, int h) {
  return w >= 800 && h >= 600 && w <= 4096 && h <= 4096;
}

#endif
