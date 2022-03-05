#pragma once
#include "graph.h"

// A helper struct to generate all edge sets to add, when a new vertex is added to an existing
// graph.
struct EdgeGenerator {
 private:
  static constexpr int MAX_EDGES = 35;  // This is the highest number we'll encounter, $\binom73$.
  int n;                                // Number of vertices after adding the new vertex.
  uint32 current_mask;                  // The binary mask indicating the current edge set.
  std::vector<uint8> edge_candidates;   // All possible edges going through the new vertex.

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
};
