#pragma once
#include "graph.h"

// A helper struct to generate all edge sets to add, when a new vertex is added to an existing
// graph.
class EdgeGenerator {
 private:
  const bool use_automorphism_opt;     // Use the automorphism optimization if this flag is true.
  const uint8 k;                       // Number of vertices in each edge.
  const uint8 n;                       // Number of vertices after adding the new vertex.
  uint8 high_idx_non_zero_enum_state;  // The highest index of the non-zero element in enum_state.
  uint8 edge_candidate_count;          // Number of edge candidates in the next array.
  uint8 edge_candidates[MAX_EDGES];    // All possible edges going through the new vertex.

  // Reverse lookup of edge_candidates. e=edge_candidates[i] iff i=edge_to_id[e]
  uint8 edge_to_id[256];

  // Whether a set of edge candidates should be tried when growing the graph.
  // The length of this array is 2^edge_candidate_count.
  // The index is the bitmask indicating which elements in edge_candidates array are in this set.
  // The boolean value indicates whether the set should be tried.
  // For example, edge_candidate_id_sets[0b0010001]=true means the set of two edges,
  // {edge_candidates[0], edge_candidates[4]}, should be tried when growing the graph.
  std::vector<bool> edge_candidate_id_sets;

  // The vertex indices in the corresponding edge candidate.
  // [0] is always NOT_IN_SET, [1] is always UNDIRECTED,
  // [2] to [k+1] are the vertices in the corresponding edge.
  uint8 edge_candidates_vidx[MAX_EDGES][10];

  // This array represents the current enumeration state.
  // The values are indices into the edge_candidates_vidx arrays.
  uint8 enum_state[MAX_EDGES];

  // Print the current state of this class to console for debugging purpose.
  void print_debug() const;

 public:
  uint8 edge_count;
  Edge edges[MAX_EDGES];

  // Initializes the generator for the given new vertex count.
  // k = number of vertices in each edge.
  EdgeGenerator(int k, int vertex_count, bool use_automorphism_optimization = false);

  // Resets the enumeration state. Starts over.
  void reset_enumeration();

  // Generates the next edge set. Returns true if the next edge set is available in `edges`.
  // Returns false if all possibilities have already been enumerated.
  bool next();

  // Notify the generator about the fact that adding the current edge set to the graph
  // makes it contain T_k, and therefore we can skip edge sets that are supersets of the current.
  void notify_contain_tk_skip();

  // Notify the generator that the given permutation is an automorphism in the base graph,
  // and therefore all edge sets that that are equivalent under this permutation can be collapsed
  // to a single try.
  void notify_automorphism(int perm[MAX_VERTICES]);
};
