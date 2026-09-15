#include "pcg_basic.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cluster.h"
#include "clusterdelta.h"
#include "config.h"
#include "particle.h"
#include "render.h"
#include "rng.h"
#include "spawn.h"
#include "stats.h"

int main(void) {
  pcg32_random_t rng1;
  InitRNG(&rng1);

  const size_t cluster_size = WIDTH * HEIGHT;

  const size_t cluster_center = ClusterIndexFromCoords((WIDTH / 2), (HEIGHT / 2));
  bool *cluster = calloc(cluster_size, sizeof(bool));
  if (cluster == NULL) {
    perror("could not allocate memory for cluster");
    return -1;
  }

  struct clusterDeltas cluster_deltas = ClusterDeltasCreate(500);

  cluster[cluster_center] = 1;
  cluster_deltas.m_RecordClusterDelta(&cluster_deltas, cluster_center, cluster_center);

  struct stats cluster_stats = StatsCreate();

  int spawn_site = INT_MAX;
  printf("1. Side-uniform Distribution\n2. Pixel-uniform Distribution\n3. Circle Circumference-uniform Distribution\n");

  while (spawn_site != 1 && spawn_site != 2 && spawn_site != 3) {
    if (scanf("%d", &spawn_site) == 1) {
      int c;
      while((c = getchar()) != '\n' && c != EOF) {};
    };
  }

  double radius = (spawn_site == 3) ? DEFAULT_RADIUS : -1.00;
  for (size_t i = 1; i <= PARTICLE_COUNT; i++) {
    struct point p;
    switch (spawn_site) {
    case 1:
      SpawnSideUniform(&p, &rng1);
      break;

    case 2:
      SpawnPixelUniform(&p, &rng1);
      break;

    case 3:
      SpawnCircumferenceUniform(&p, radius, cluster_center, &rng1);
      break;
    }

    cluster_deltas.m_RecordClusterDelta(&cluster_deltas, ClusterIndexFromCoords(p.x, p.y), ClusterIndexFromCoords(p.x, p.y));
    cluster_stats.m_total_particles++;

    Walk(&cluster_deltas, &cluster_stats, p, cluster_center, &radius, cluster, spawn_site, &rng1);
  }

  printf("Reached the End of Simulation\n");

  if (Render(cluster) == 1) {
    return 1;
  }

  printf("Rendered Final State of DLA run\n");

  if (AnimateCluster(&cluster_deltas) == 1) {
    return 1;
  }

  printf("Animated DLA run\n");

  free(cluster);
  ClusterDeltasDestroy(&cluster_deltas);
  StatsDestroy(&cluster_stats);

  return 0;
}
