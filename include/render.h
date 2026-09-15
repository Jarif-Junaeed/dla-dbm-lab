#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

#include "clusterdelta.h"

int Render(bool *cluster);
int AnimateCluster(struct clusterDeltas *cluster_deltas);

#endif
