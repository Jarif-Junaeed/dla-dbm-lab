#ifndef PARTICLE_H
#define PARTICLE_H

#include <stdbool.h>
#include <stddef.h>

#include "cluster.h"
#include "clusterdelta.h"
#include "pcg_basic.h"
#include "stats.h"

void Walk(struct clusterDeltas *cluster_deltas, struct stats *cluster_stats, struct point p, size_t cluster_center, double *radius, bool *cluster, pcg32_random_t *rng);

#endif
