#pragma once

#include "edge_gen.h"
#include "graph.h"

// Grow set of non-isomorphic graphs from empty graph, by adding one vertex at a time.
class Grower {
 private:
  // Custom hash and compare for the Graph type. Treat isomorphic graphs as being equal.
  struct GraphHasher {
    size_t operator()(const Graph& g) const { return g.graph_hash; }
  };
  struct GraphComparer {
    bool operator()(const Graph& g, const Graph& h) const { return g.is_isomorphic(h); }
  };

  // The log file.
  std::ostream* const log;
  // Whether to use codegree info in computing graph hash. And if yes, the number of vertices
  // used in the codegree computation. 0 means not to use codegree info.
  const int use_codegrees;
  // Utility used to enumerate all edge sets to add.
  EdgeGenerator edge_gen;

  // Constructs all non-isomorphic graphs with n vertices that are T_k-free,
  // and add them to the canonicals. Before calling this, all such graphs
  // with <n vertices must already be in the canonicals.
  // Note all edges added in this step contains vertex (n-1).
  void grow_step(int n);

  void print_config(std::ostream& os);
  void print_state_to_stream(bool print_graphs, std::ostream& os);

 public:
  // Constructs the Grower object.
  //
  // The codegrees parameter specifies whether to use codegree info in computing graph hash.
  // And if yes, the number of vertices used in the codegree computation. 0 means not to use
  // codegree info.
  // log_stream is used for status reporting and debugging purpose.
  Grower(int codegrees, std::ostream* log_stream = nullptr);

  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  std::unordered_set<Graph, GraphHasher, GraphComparer> canonicals[MAX_VERTICES + 1];

  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow();

  // Debug print the content of the canonicals after the growth.
  // If print_graphs==true, print stats and all graphs. Otherwise prints stats only.
  void print(bool print_graphs);
};