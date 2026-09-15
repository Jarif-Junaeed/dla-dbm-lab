#include "walker.h"

#include <math.h>

#include "config.h"
#include "grid.h"

static double EuclideanDistance(double x1, double y1, double x2, double y2) {
  double delx = x2 - x1;
  double dely = y2 - y1;

  return sqrt((delx * delx) + (dely * dely));
}

void Walk(struct gridDeltas *grid_deltas, struct stats *grid_stats, struct point p, size_t grid_center, double *radius, bool *grid, pcg32_random_t *rng) {
  for (size_t i = 0; i < MAX_WALKER_STEPS; i++) {
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

    // Walker went out of bounds
    if (next_x < 0 || next_x >= WIDTH || next_y < 0 || next_y >= HEIGHT) {
      grid_deltas->m_ReduceGridDeltasUsedCapacity(grid_deltas, i+1);
      return;
    }

    // Walker sticks to the aggregate
    if ((next_y > 0 && grid[GridIndexFromCoords(next_x, next_y - 1)]) ||
        (next_y < HEIGHT - 1 && grid[GridIndexFromCoords(next_x, next_y + 1)]) ||
        (next_x > 0 && grid[GridIndexFromCoords(next_x - 1, next_y)]) ||
        (next_x < WIDTH - 1 && grid[GridIndexFromCoords(next_x + 1, next_y)])) {

      grid[GridIndexFromCoords(next_x, next_y)] = 1;
      grid_deltas->m_RecordGridDelta(grid_deltas, GridIndexFromCoords(next_x, next_y), GridIndexFromCoords(p.x, p.y));

      struct point grid_center_coords = GridCoordsFromIndex(grid_center);

      grid_stats->m_walker_stats[grid_stats->m_clustered_walkers].m_walker_steps = i + 1;
      grid_stats->m_walker_stats[grid_stats->m_clustered_walkers].m_stuck_pos = (struct point){.x = next_x, .y = next_y};
      grid_stats->m_walker_stats[grid_stats->m_clustered_walkers].m_rmax = *radius;


      // We calulate the radius of gyration using rg = sqrt[{(sum_x2 + sum_y2) / N} - COM_x^2 - COM_y^2]
      // which is mathematically equivalent to rg = sqrt[sum{ (x_i - COM_x)^2 + (y_i - COM_y)^2 } / N]
      double sum_x = grid_center_coords.x;
      double sum_y = grid_center_coords.y;
      double sum_x2 = grid_center_coords.x * grid_center_coords.x;
      double sum_y2 = grid_center_coords.y * grid_center_coords.y;
      for (size_t i = 0; i < grid_stats->m_clustered_walkers + 1; i++) {
        double x = grid_stats->m_walker_stats[i].m_stuck_pos.x;
        double y = grid_stats->m_walker_stats[i].m_stuck_pos.y;

        sum_x += x;
        sum_y += y;

        sum_x2 += x * x;
        sum_y2 += y * y;
      }

      // We have a + 2 to include the seed as well
      double com_x = sum_x / (grid_stats->m_clustered_walkers + 2);
      double com_y = sum_y / (grid_stats->m_clustered_walkers + 2);

      double avg_r2 = ((sum_x2 + sum_y2) / (grid_stats->m_clustered_walkers + 2)) - (com_x * com_x) - (com_y * com_y);
      grid_stats->m_walker_stats[grid_stats->m_clustered_walkers].m_rg = sqrt(avg_r2);

      grid_stats->m_clustered_walkers++;

      double maximum_radius = (WIDTH > HEIGHT) ? (HEIGHT/ 2) : (WIDTH / 2);
      if (*radius != -1 && ((*radius) < maximum_radius)) {
        double new_radius = EuclideanDistance(grid_center_coords.x, grid_center_coords.y, next_x, next_y);
        if (new_radius > (*radius)) *radius = new_radius;
      }

      return;
    }
    // if walker does not stick to aggregate within MAX_WALKER_STEPS, then we kill it
    else if ( i == MAX_WALKER_STEPS - 1) {
      grid_deltas->m_ReduceGridDeltasUsedCapacity(grid_deltas, i+1);
      return;
    }

    // Walker moved to a valid position that is not adjacent to the aggregate within MAX_WALKER_STEPS
    grid_deltas->m_RecordGridDelta(grid_deltas, GridIndexFromCoords(next_x, next_y), GridIndexFromCoords(p.x, p.y));
    p.x = next_x;
    p.y = next_y;
  }
  return;
}
