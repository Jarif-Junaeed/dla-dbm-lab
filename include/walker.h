#ifndef WALKER_H
#define WALKER_H

#include <stdbool.h>
#include <stddef.h>

#include "griddelta.h"
#include "pcg_basic.h"
#include "stats.h"

void Walk(struct gridDeltas *grid_deltas, struct stats *grid_stats, int x, int y, size_t grid_center, double *radius, bool *grid, pcg32_random_t *rng);

#endif
