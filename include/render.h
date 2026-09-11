#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

#include "griddelta.h"

int Render(bool *grid);
int AnimateAggregation(struct gridDeltas *grid_deltas);

#endif
