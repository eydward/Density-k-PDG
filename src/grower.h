#pragma once

#include "edge_gen.h"
#include "graph.h"

// Grow set of non-isomorphic graphs from empty graph, by adding one vertex at a time.
class Grower {
 private:
  // Custom hash and compare for the Graph type. Treat isomorphic graphs as being equal.
  struct GraphHasher {
    size_t operator()(const Graph& g) const { return g.get_graph_hash(); }
  };
  struct GraphComparer {
    bool operator()(const Graph& g, const Graph& h) const { return g.is_isomorphic(h); }
  };

  // Whether to print the graph content after the growth phase.
  const bool print_graph;
  // The log file.
  std::ostream* const log;
  // Utility used to enumerate all edge sets to add.
  EdgeGenerator edge_gen;

  // Constructs all non-isomorphic graphs with n vertices that are T_k-free,
  // and add them to the canonicals. Before calling this, all such graphs
  // with <n vertices must already be in the canonicals.
  // Note all edges added in this step contains vertex (n-1).
  // This is used to grow all graphs up to N-1 vertices.
  void grow_step(int n);

  // Enumerate all graphs in the final step where all graphs have N vertices.
  // We don't need to store any graph in this step.
  void enumerate_final_step();

  void print_config(std::ostream& os);
  void print_state_to_stream(bool print_graphs, std::ostream& os);

 public:
  // Constructs the Grower object.
  // log_stream is used for status reporting and debugging purpose.
  Grower(bool print_graph_ = false, std::ostream* log_stream = nullptr);

  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  std::unordered_set<Graph, GraphHasher, GraphComparer> canonicals[MAX_VERTICES];

  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow();

  // Debug print the content of the canonicals after the growth.
  // If print_graphs==true, print stats and all graphs. Otherwise prints stats only.
  void print(bool print_graphs);
};