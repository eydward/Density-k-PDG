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

  // Generates a new graph in `copy` by:
  //   (1) copies the base graph into `copy`,
  //   (2) adds the edges specified by the current enumeration state.
  // If skip_front>0, the first `skip_front` number of edges in the enumeration state
  // are skipped and not added to the new graph.
  void generate_graph(Graph& copy, int skip_front) const;

  // Performs the min_theta optimization, and returns one of the three results.
  //
  // Idea of the min_theta optimization: given the base graph and the current enum state,
  // we know what theta value of the new graph, without generating the new graph. If the
  // theta value is >= the currently known min_theta value, then the new graph is not interesting
  // to us, regardless whether it's T_k free, because it won't be able to lower the min_theta
  // value anyway. So in such case, we can advance the enumeration state to the next possible
  // state that may give us a smaller theta value, skipping many states to make the algorithm
  // computationally feasible. This is very important when the base graph is very sparse
  // and therefore the new graphs are less likely to contain T_k, making the
  // `notify_contain_tk_skip` optimization less effective.
  enum class OptResult { FOUND_CANDIDATE = 0, CONTINUE_SEARCH = 1, DONE = 2 };
  OptResult perform_min_theta_optimization(Fraction known_min_theta);

 public:
  // Some performance counters used to monitor the edge gen behavior.
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
  // known_min_theta = the currently know min_theta value.
  //
  // The min_theta optimization: this should only be used in the final enumeration phase, not
  // in the growth phase before the final step.
  //
  // The idea is, if the graph is too sparse, then its theta is guaranteed to be larger than
  // the currently known min_theta value, in which case we don't care about this graph since
  // it won't give us a better min_theta value regardless whether the graph is T_k free.
  bool next(Graph& copy, bool use_known_min_theta_opt = false,
            Fraction known_min_theta = Fraction(0, 1));

  // Notify the generator about the fact that adding the current edge set to the graph
  // makes it contain T_k, and therefore we can skip edge sets that are supersets of the current.
  void notify_contain_tk_skip();

  // Print the current state of this class to the given stream for debugging purpose.
  void print_debug(std::ostream& os, bool print_candidates, int base_graph_id) const;
};
