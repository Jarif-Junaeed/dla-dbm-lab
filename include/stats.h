#ifndef STATS_H
#define STATS_H

#include <stddef.h>

struct walkerStats {
  double m_stuck_pos_x;
  double m_stuck_pos_y;
  size_t m_walker_steps;
  size_t m_rmax;
  size_t m_rg;
};

struct stats {
  size_t m_total_walkers;
  size_t m_clustered_walkers;
  struct walkerStats *m_walker_stats;
};

struct stats StatsCreate(void);
void StatsDestroy(struct stats *stats);

#endif
