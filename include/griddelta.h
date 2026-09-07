#ifndef GRIDDELTA_H
#define GRIDDELTA_H

#include <stdbool.h>
#include <stddef.h>

struct gridDelta {
  size_t dest;
  size_t src;
};

struct gridDeltas {
  size_t m_capacity;
  size_t m_used_capacity;
  struct gridDelta *m_grid_deltas;

  void (*m_RecordGridDelta)(struct gridDeltas *self, size_t dest, size_t src);
};

struct gridDeltas GridDeltasCreate(size_t capacity);
void GridDeltasDestroy(struct gridDeltas *self);

#endif
