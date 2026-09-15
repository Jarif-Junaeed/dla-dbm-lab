#include "stats.h"

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

struct stats StatsCreate(void) {
  struct particleStats *particle_stats = malloc(PARTICLE_COUNT * sizeof(struct particleStats));
  if (particle_stats == NULL) {
    perror("could not allocate memory for particle_stats");
    exit(1);
  }

  struct stats result = (struct stats){.m_total_particles = 0, .m_clustered_particles = 0, .m_particle_stats = particle_stats};

  return result;
}

void StatsDestroy(struct stats *stats) {
  free(stats->m_particle_stats);
  stats->m_particle_stats = NULL;
  stats->m_total_particles = 0;
  stats->m_clustered_particles = 0;
}
