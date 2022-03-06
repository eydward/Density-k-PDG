#pragma once
#include "graph.h"

// A helper struct to generate all edge sets to add, when a new vertex is added to an existing
// graph.
class EdgeGenerator {
 private:
  uint8 k;                             // Number of vertices in each edge.
  uint8 n;                             // Number of vertices after adding the new vertex.
  uint8 high_idx_non_zero_enum_state;  // The highest index of the non-zero element in enum_state.
  uint8 edge_candidate_count;          // Number of edge candidates in the next array.
  uint8 edge_candidates[MAX_EDGES];    // All possible edges going through the new vertex.

  // The vertex indices in the corresponding edge candidate.
  // [0] is always NOT_IN_SET, [1] is always UNDIRECTED,
  // [2] to [k+1] are the vertices in the corresponding edge.
  uint8 edge_candidates_vidx[MAX_EDGES][10];

  // This array represents the current enumeration state.
  // The values are indices into the edge_candidates_vidx arrays.
  uint8 enum_state[MAX_EDGES];

 public:
  uint8 edge_count;
  Edge edges[MAX_EDGES];

  // Initializes the generator for the given new vertex count.
  // k = number of vertices in each edge.
  void initialize(int k, int vertex_count);

  // Resets the enumeration state. Starts over.
  void reset_enumeration();

  // Generates the next edge set. Returns true if the next edge set is available in `edges`.
  // Returns false if all possibilities have already been enumerated.
  bool next();

  // Notify the generator about the fact that adding the current edge set to the graph
  // makes it contain T_k, and therefore we can skip edge sets that are supersets of the current.
  void notify_contain_tk_skip();
};
