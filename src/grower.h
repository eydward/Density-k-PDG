#pragma once

#include "edge_gen.h"
#include "graph.h"

// Grow set of non-isomorphic graphs from empty graph, by adding one vertex at a time.
class Grower {
 private:
  // The number of worker threads to use in the final enumeration step.
  const int num_worker_threads;
  // If true, stop after the growing phase but don't perform the final enumeration phase.
  const bool skip_final_enum;
  // The start index and end index in the final enumeration phase.
  const int start_idx;
  const int end_idx;
  // The log files.
  std::ostream* const log;
  std::ostream* const log_detail;

  // Returns a collection of graphs with n vertices that are T_k-free, one in each
  // isomorphism class. Note all edges added in this step contains vertex (n-1).
  // The second parameter is the collection of graphs collected from the previous step
  // with (n-1) vertices.
  //
  // This function is called repeatedly to grow all graphs up to N-1 vertices.
  std::vector<Graph> grow_step(int n, const std::vector<Graph>&);

  // Enumerates all graphs in the final step where all graphs have N vertices.
  // We don't need to collect any graph in this step.
  // The parameter is the collection of graphs collected from the last grow_step()
  // with (N-1) vertices.
  void enumerate_final_step(const std::vector<Graph>&);

  // Prints the content of the canonicals after the growth to console and log files.
  void print_before_final(const std::vector<Graph> collected_graphs[MAX_VERTICES]) const;
  void print_state_to_stream(std::ostream& os,
                             const std::vector<Graph> collected_graphs[MAX_VERTICES]) const;
  // Prints the configuration to the given output stream.
  void print_config(std::ostream& os) const;

  // The entry point of the worker thread, used in the final enumeration phase.
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
  Grower(int num_worker_threads_, int skip_final_enum_, int start_idx_, int end_idx_,
         std::ostream* log_ = nullptr, std::ostream* log_detail_ = nullptr);

  // Finds all canonical isomorphism class representations with up to max_n vertices.
  void grow();
};