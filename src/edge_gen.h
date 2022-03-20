#pragma once
#include "graph.h"

// A helper struct to generate all edge sets to add, when a new vertex is added to an existing
// graph.
class EdgeGenerator {
 private:
  const Graph& base;                   // The base graph to grow from.
  const uint8 n;                       // Number of vertices after adding the new vertex.
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

  // Returns a tuple:
  //    first = number of edges in the current state,
  //    second = number of directed edges in the current state,
  //    third = lowest index in enum_state where the edge is not in the set.
  //    forth = lowest index in enum_state where the edge is undirected or not in the set.
  std::tuple<uint8, uint8, uint8, uint8> count_edges() const;

  enum class OptResult { FOUND_CANDIDATE = 0, CONTINUE_SEARCH = 1, DONE = 2 };
  OptResult perform_min_theta_optimization(int base_edge_count, int base_directed_edge_count,
                                           Fraction known_min_theta);

  // The generated edges in the current state.
  uint8 edge_count;
  Edge edges[MAX_EDGES];

 public:
  uint64 stats_tk_skip;           // How many notify_contain_tk_skip().
  uint64 stats_tk_skip_bits;      // How many bits in total did notify_contain_tk_skip() skip.
  uint64 stats_theta_edges_skip;  // How many skips due to min_theta opt, not enough edges.
  uint64 stats_theta_directed_edges_skip;  // How many skips due to min_theta opt, directed.
  uint64 stats_edge_sets;                  // How many edge sets returned from this generator.
  void clear_stats();                      // Clear the above stats.

  // Initializes the generator for the given new vertex count.
  // vertex_count = number of vertices to grow to in each new graph.
  EdgeGenerator(int vertex_count, const Graph& base_graph);

  // Generates the next edge set. Returns true enumeration should proceed,
  // in which case `copy` is the newly generated graph. (`copy` doesn't need to be clean before
  // calling this function, all its state will be reset.)
  // Returns false if all possibilities have already been enumerated.
  //
  // use_known_min_theta_opt = whether min_theta optimization should be used. If false,
  //    the rest of the paramters are all ignored.
  // base_edge_count = number of edges in the base graph.
  // base_directed_edge_count = number of directed edges in the base graph.
  // known_min_theta = the currently know min_theta value.
  //
  // The min_theta optimization: this should only be used in the final enumeration phase, not
  // in the growth phase before the final step.
  //
  // The idea is, if the graph is too sparse, then its theta is guaranteed to be larger than
  // the currently known min_theta value, in which case we don't care about this graph since
  // it won't give us a better min_theta value regardless whether the graph is T_k free.
  bool next(Graph& copy, bool use_known_min_theta_opt = false, int base_edge_count = -1,
            int base_directed_edge_count = -1, Fraction known_min_theta = Fraction(0, 1));

  // Notify the generator about the fact that adding the current edge set to the graph
  // makes it contain T_k, and therefore we can skip edge sets that are supersets of the current.
  void notify_contain_tk_skip();

  // Print the current state of this class to console for debugging purpose.
  void print_debug(bool print_candidates) const;
};
