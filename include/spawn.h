#ifndef SPAWN_H
#define SPAWN_H

#include <stddef.h>

#include "pcg_basic.h"

void SpawnSideUniform(int *x, int *y, pcg32_random_t *rng);
void SpawnPixelUniform(int *x, int *y, pcg32_random_t *rng);
void SpawnCircumferenceUniform(int *x, int *y, double radius, size_t center, pcg32_random_t *rng);

#endif
