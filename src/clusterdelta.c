#include "clusterdelta.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>

void RecordClusterDelta(struct clusterDeltas *self, size_t dest, size_t src) {

  if (self->m_capacity == 0) {
    perror("RecordClusterDelta() called on a clusterDeltas with 0 capacity");
    exit(1);
  }

  if (self->m_used_capacity >= self->m_capacity) {
    self->m_capacity *= 2;
    struct clusterDelta *tmp = realloc(self->m_cluster_deltas, self->m_capacity * sizeof(struct clusterDelta));
    if (tmp == NULL) {
      perror("clusterDelta realloc failed");
      exit(1);
    }
    self->m_cluster_deltas = tmp;
  }
  (self->m_cluster_deltas)[self->m_used_capacity++] = (struct clusterDelta){.dest = dest, .src = src};
}

void ReduceClusterDeltasUsedCapacity(struct clusterDeltas *self, size_t amount) {
  if(amount > self->m_used_capacity) {
    perror("Cannot reduce used_capacity to below 0");
    exit(1);
  } else {
    self->m_used_capacity -= amount;
  }
}

struct clusterDeltas ClusterDeltasCreate(size_t capacity) {

  if (capacity <= 0) {
    capacity = 100; // default capacity
  }

  struct clusterDelta *cluster_deltas = malloc(capacity * sizeof(struct clusterDelta));
  if (cluster_deltas == NULL) {
    perror("could not allocate memory for cluster");
    exit(1);
  }
  struct clusterDeltas result = (struct clusterDeltas){.m_capacity = capacity, .m_used_capacity = 0,
                                                 .m_cluster_deltas = cluster_deltas,
                                                 .m_RecordClusterDelta = RecordClusterDelta,
                                                 .m_ReduceClusterDeltasUsedCapacity = ReduceClusterDeltasUsedCapacity};

  return result;
}

void ClusterDeltasDestroy(struct clusterDeltas *self) {
  free(self->m_cluster_deltas);
  self->m_cluster_deltas = NULL;
  self->m_used_capacity = 0;
  self->m_capacity = 0;
}
