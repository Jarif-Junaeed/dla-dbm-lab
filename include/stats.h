#ifndef STATS_H
#define STATS_H

#include <stddef.h>

#include "cluster.h"

struct particleStats {
  struct point m_stuck_pos;
  size_t m_particle_steps;
  size_t m_rmax;
  double m_rg;
};

struct stats {
  size_t m_total_particles;
  size_t m_clustered_particles;
  struct particleStats *m_particle_stats;
};

struct stats StatsCreate(void);
void StatsDestroy(struct stats *stats);

#endif
