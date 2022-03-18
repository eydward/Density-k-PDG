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

  // The number of worker threads to use in the final enumeration step.
  const int num_worker_threads;
  // The starting index in the final enumeration phase.
  const int restart_idx;
  // The log files.
  std::ostream* const log;
  std::ostream* const log_detail;
  // The worker threads used in the final enumeration phase.
  std::vector<std::thread> worker_threads;

  // Constructs all non-isomorphic graphs with n vertices that are T_k-free,
  // and add them to the canonicals. Before calling this, all such graphs
  // with <n vertices must already be in the canonicals.
  // Note all edges added in this step contains vertex (n-1).
  // This is used to grow all graphs up to N-1 vertices.
  void grow_step(int n);

  // Enumerate all graphs in the final step where all graphs have N vertices.
  // We don't need to store any graph in this step.
  void enumerate_final_step();

  // Print the content of the canonicals after the growth to console and log files.
  void print_before_final() const;
  void print_config(std::ostream& os) const;
  void print_state_to_stream(std::ostream& os) const;

  void worker_thread_main(int thread_id);

  // The mutex to protect the counters under multi-threading.
  std::mutex counters_mutex;
  // The mutex to protect the queue under multi-threading.
  std::mutex queue_mutex;
  // The queue of base graphs to be processed in the final enumeration phase.
  // The worker threads will dequeue graphs from this queue to work on.
  std::queue<Graph> to_be_processed;
  // The number of graphs that have been dequeued from the to_be_processed queue.
  int to_be_processed_id;

 public:
  // Constructs the Grower object.
  // log_stream is used for status reporting and debugging purpose.
  Grower(int num_worker_threads_ = 0, int restart_idx_ = 0, std::ostream* log_ = nullptr,
         std::ostream* log_detail_ = nullptr);

  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  std::unordered_set<Graph, GraphHasher, GraphComparer> canonicals[MAX_VERTICES];

  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow();
};