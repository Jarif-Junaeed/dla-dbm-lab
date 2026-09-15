#ifndef GRID_H
#define GRID_H

#include <stddef.h>

#include "config.h"

struct point {
  size_t x;
  size_t y;
};

static inline size_t PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

static inline size_t GridIndexFromCoords(int x, int y) { return (size_t)y * WIDTH + (size_t)x; }
static inline struct point GridCoordsFromIndex(size_t index) {
  return (struct point){.x = index % WIDTH, .y = index / WIDTH};
}

#endif
