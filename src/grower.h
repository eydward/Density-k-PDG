#pragma once

#include "edge_gen.h"
#include "graph.h"

// Grow set of non-isomorphic graphs from empty graph, by adding one vertex at a time.
template <int K, int N>
class Grower {
 private:
  // Alias of the Graph type
  typedef Graph<K, N> G;

  // Custom hash and compare for the Graph type. Treat isomorphic graphs as being equal.
  struct GraphHasher {
    size_t operator()(const G& g) const { return g.hash; }
  };
  struct GraphComparer {
    bool operator()(const G& g, const G& h) const { return g.is_isomorphic(h); }
  };

  // The log file.
  std::ostream* const log;
  // Utility used to enumerate all edge sets to add.
  EdgeGenerator edge_gen;

  // Constructs all non-isomorphic graphs with n vertices that are T_k-free,
  // and add them to the canonicals. Before calling this, all such graphs
  // with <n vertices must already be in the canonicals.
  // Note all edges added in this step contains vertex (n-1).
  void grow_step(int n);

  // This is an alternative implementation of grow_step() that uses a different growth strategy.
  void grow_step_impl_alt(int n);

  void print_config(std::ostream& os);
  void print_state_to_stream(bool print_graphs, std::ostream& os);

 public:
  Grower(std::ostream* log_stream = nullptr) : log(log_stream) {}

  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  std::unordered_set<G, GraphHasher, GraphComparer> canonicals[N + 1];

  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow(bool alt_growth_strategy);

  // Debug print the content of the canonicals after the growth.
  // If print_graphs==true, print stats and all graphs. Otherwise prints stats only.
  void print(bool print_graphs);
};