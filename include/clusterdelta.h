#ifndef CLUSTERDELTA_H
#define CLUSTERDELTA_H

#include <stdbool.h>
#include <stddef.h>

struct clusterDelta {
  size_t dest;
  size_t src;
};

struct clusterDeltas {
  size_t m_capacity;
  size_t m_used_capacity;
  struct clusterDelta *m_cluster_deltas;

  void (*m_RecordClusterDelta)(struct clusterDeltas *self, size_t dest, size_t src);
  void (*m_ReduceClusterDeltasUsedCapacity)(struct clusterDeltas *self, size_t amount);
};

struct clusterDeltas ClusterDeltasCreate(size_t capacity);
void ClusterDeltasDestroy(struct clusterDeltas *self);

#endif
