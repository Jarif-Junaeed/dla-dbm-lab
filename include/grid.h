#ifndef GRID_H
#define GRID_H

#include <stddef.h>
#include <stdint.h>

#include "config.h"

static inline size_t PixelIndex(int pixel) { return pixel * COLOR_CHANNELS; }

static inline size_t GridIndexFromCoords(int x, int y) { return (size_t)y * WIDTH + (size_t)x; }
static inline uint64_t GridCoordsFromIndex(size_t index) {
  int y = index / WIDTH;
  int x = index % WIDTH;

  uint64_t coords = 0;
  coords = (coords | x) << 32;
  coords = (coords | y);

  return coords;
}

#endif
