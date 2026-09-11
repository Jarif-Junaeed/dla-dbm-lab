#include "stats.h"

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

struct stats StatsCreate(void) {
  struct walkerStats *walker_stats = malloc(WALKER_COUNT * sizeof(struct walkerStats));
  if (walker_stats == NULL) {
    perror("could not allocate memory for walker_stats");
    exit(1);
  }

  struct stats result = (struct stats){.m_total_walkers = 0, .m_clustered_walkers = 0, .m_walker_stats = walker_stats};

  return result;
}

void StatsDestroy(struct stats *stats) {
  free(stats->m_walker_stats);
  stats->m_walker_stats = NULL;
  stats->m_total_walkers = 0;
  stats->m_clustered_walkers = 0;
}
