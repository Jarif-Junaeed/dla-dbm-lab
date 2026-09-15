#include "particle.h"

#include <math.h>
#include <stdint.h>

#include "cluster.h"
#include "config.h"
#include "pcg_basic.h"
#include "spawn.h"

static double EuclideanDistance(double x1, double y1, double x2, double y2) {
  double delx = x2 - x1;
  double dely = y2 - y1;

  return sqrt((delx * delx) + (dely * dely));
}

void Walk(struct clusterDeltas *cluster_deltas, struct stats *cluster_stats, struct point p, size_t cluster_center, double *radius, bool *cluster, int spawn_site, pcg32_random_t *rng) {
  size_t deltas_recorded = 1; // includes the spawn delta
  for (size_t i = 0; i < MAX_PARTICLE_STEPS; i++) {
    unsigned int direction = pcg32_random_r(rng) & 3;
    long next_x = p.x;
    long next_y = p.y;
    switch (direction) {
    case 0:
      next_y--;
      break;
    case 1:
      next_x++;
      break;
    case 2:
      next_y++;
      break;
    case 3:
      next_x--;
      break;
    }

    bool rand_stick = (((double)pcg32_random_r(rng) / UINT32_MAX) <= DLA_STICKINESS_FACTOR) ? 1 : 0;

    // We respawn particle if it goes out of bounds
    if (next_x < 0 || next_x >= WIDTH || next_y < 0 || next_y >= HEIGHT) {
      struct point tmp_p = p;
      switch (spawn_site) {
      case 1:
        SpawnSideUniform(&p, rng);
        break;

      case 2:
        SpawnPixelUniform(&p, rng);
        break;

      case 3:
        SpawnCircumferenceUniform(&p, *radius, cluster_center, rng);
        break;
      }
      
      cluster_deltas->m_RecordClusterDelta(cluster_deltas, ClusterIndexFromCoords(p.x, p.y), ClusterIndexFromCoords(tmp_p.x, tmp_p.y));
      deltas_recorded++;
      continue;
    }

    // Particle sticks to the cluster if rand_stick is 1 
    if ((next_y > 0 && cluster[ClusterIndexFromCoords(next_x, next_y - 1)]) ||
        (next_y < HEIGHT - 1 && cluster[ClusterIndexFromCoords(next_x, next_y + 1)]) ||
        (next_x > 0 && cluster[ClusterIndexFromCoords(next_x - 1, next_y)]) ||
        (next_x < WIDTH - 1 && cluster[ClusterIndexFromCoords(next_x + 1, next_y)])) {

      if (!rand_stick) {
        cluster_deltas->m_RecordClusterDelta(
            cluster_deltas, ClusterIndexFromCoords(next_x, next_y),
            ClusterIndexFromCoords(p.x, p.y));
        cluster_deltas->m_RecordClusterDelta(
            cluster_deltas, ClusterIndexFromCoords(p.x, p.y),
            ClusterIndexFromCoords(next_x, next_y));
        deltas_recorded += 2;
        continue;
      }

      cluster[ClusterIndexFromCoords(next_x, next_y)] = 1;
      cluster_deltas->m_RecordClusterDelta(cluster_deltas, ClusterIndexFromCoords(next_x, next_y), ClusterIndexFromCoords(p.x, p.y));

      struct point cluster_center_coords = ClusterCoordsFromIndex(cluster_center);

      cluster_stats->m_particle_stats[cluster_stats->m_clustered_particles].m_particle_steps = i + 1;
      cluster_stats->m_particle_stats[cluster_stats->m_clustered_particles].m_stuck_pos = (struct point){.x = next_x, .y = next_y};
      cluster_stats->m_particle_stats[cluster_stats->m_clustered_particles].m_rmax = *radius;


      // We calulate the radius of gyration using rg = sqrt[{(sum_x2 + sum_y2) / N} - COM_x^2 - COM_y^2]
      // which is mathematically equivalent to rg = sqrt[sum{ (x_i - COM_x)^2 + (y_i - COM_y)^2 } / N]
      double sum_x = cluster_center_coords.x;
      double sum_y = cluster_center_coords.y;
      double sum_x2 = cluster_center_coords.x * cluster_center_coords.x;
      double sum_y2 = cluster_center_coords.y * cluster_center_coords.y;
      for (size_t i = 0; i < cluster_stats->m_clustered_particles + 1; i++) {
        double x = cluster_stats->m_particle_stats[i].m_stuck_pos.x;
        double y = cluster_stats->m_particle_stats[i].m_stuck_pos.y;

        sum_x += x;
        sum_y += y;

        sum_x2 += x * x;
        sum_y2 += y * y;
      }

      // We have a + 2 to include the seed as well
      double com_x = sum_x / (cluster_stats->m_clustered_particles + 2);
      double com_y = sum_y / (cluster_stats->m_clustered_particles + 2);

      double avg_r2 = ((sum_x2 + sum_y2) / (cluster_stats->m_clustered_particles + 2)) - (com_x * com_x) - (com_y * com_y);
      cluster_stats->m_particle_stats[cluster_stats->m_clustered_particles].m_rg = sqrt(avg_r2);

      cluster_stats->m_clustered_particles++;

      double maximum_radius = (WIDTH > HEIGHT) ? (HEIGHT/ 2) : (WIDTH / 2);
      if (*radius != -1 && ((*radius) < maximum_radius)) {
        double new_radius = EuclideanDistance(cluster_center_coords.x, cluster_center_coords.y, next_x, next_y);
        if (new_radius > (*radius)) *radius = new_radius;
      }

      return;
    }

    // Particle moved to a valid position that is not adjacent to the cluster within MAX_PARTICLE_STEPS
    cluster_deltas->m_RecordClusterDelta(cluster_deltas, ClusterIndexFromCoords(next_x, next_y), ClusterIndexFromCoords(p.x, p.y));
    deltas_recorded++;
    p.x = next_x;
    p.y = next_y;
  }
  cluster_deltas->m_ReduceClusterDeltasUsedCapacity(cluster_deltas, deltas_recorded);
  return;
}
