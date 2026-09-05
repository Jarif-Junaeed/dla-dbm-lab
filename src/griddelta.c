#include "griddelta.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>

void RecordGridDelta(struct gridDeltas *self, int dest, int src) {
  
  if (self->m_capacity == 0) {
    perror("RecordGridDelta() called on a gridDeltas with 0 capacity");
    exit(1);
  } 

  if (self->m_used_capacity >= self->m_capacity) {
    self->m_capacity *= 2; 
    struct gridDelta *tmp = realloc(
        self->m_grid_deltas, self->m_capacity * sizeof(struct gridDelta));
    if (tmp == NULL) {
      perror("gridDelta realloc failed");
      exit(1);
    }
    self->m_grid_deltas = tmp;
  }
  (self->m_grid_deltas)[self->m_used_capacity++] = (struct gridDelta){.dest = dest, .src = src};
}

struct gridDeltas GridDeltasCreate(size_t capacity) {

  if (capacity <= 0) {
    capacity = 100; // default capacity
  }

  struct gridDelta *grid_deltas = malloc(capacity * sizeof(struct gridDelta));
  if (grid_deltas == NULL) {
    perror("could not allocate memory for grid");
    exit(1);
  }
  struct gridDeltas result = (struct gridDeltas){.m_capacity = capacity, .m_used_capacity = 0,
                                                 .m_grid_deltas = grid_deltas,
                                                 .m_RecordGridDelta = RecordGridDelta};

  return result;
}

void GridDeltasDestroy(struct gridDeltas *self) {
  free(self->m_grid_deltas);
  self->m_grid_deltas = NULL;
  self->m_used_capacity = 0;
  self->m_capacity = 0;
}
