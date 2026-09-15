#ifndef SPAWN_H
#define SPAWN_H

#include <stddef.h>

#include "grid.h"
#include "pcg_basic.h"

void SpawnSideUniform(struct point *p, pcg32_random_t *rng);
void SpawnPixelUniform(struct point *p, pcg32_random_t *rng);
void SpawnCircumferenceUniform(struct point *p, double radius, size_t center, pcg32_random_t *rng);

#endif
