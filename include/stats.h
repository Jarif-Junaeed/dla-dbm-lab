#ifndef STATS_H
#define STATS_H

#include <stddef.h>

#include "grid.h"

struct walkerStats {
  struct point m_stuck_pos;
  size_t m_walker_steps;
  size_t m_rmax;
  double m_rg;
};

struct stats {
  size_t m_total_walkers;
  size_t m_clustered_walkers;
  struct walkerStats *m_walker_stats;
};

struct stats StatsCreate(void);
void StatsDestroy(struct stats *stats);

#endif
