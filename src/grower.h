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
  // If true, the min_theta optimization is enabled.
  const bool use_min_theta_opt;
  // If true, the contains T_k optimization is enabled.
  const bool use_contains_Tk_opt;
  // The start index and end index in the final enumeration phase.
  const int start_idx;
  const int end_idx;
  // If true, search for all graphs that generate the given ratio value,
  // instead of searching for min_ratio.
  const bool search_ratio_graph;
  // The ratio value to search for. Ignored unless search_ratio_graph==true.
  const Fraction ratio_to_search;

  // The number of generations between checking whether should print statistics.
  uint64 stats_check_every_n_gen = 100000;
  // The number of seconds between printing statistics in the final enumeration step.
  int stats_print_every_n_seconds = 20;

  // The log files.
  std::ostream* log;
  std::ostream* log_detail;
  std::ostream* log_result;

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

  // The entry point of the worker thread, used in the final enumeration phase.
  void worker_thread_main(int thread_id);

  // The mutex to protect the counters under multi-threading.
  std::mutex counters_mutex;
  // The mutex to protect the queue and results under multi-threading.
  std::mutex queue_mutex;
  // The queue of base graphs to be processed in the final enumeration phase.
  // The worker threads will dequeue graphs from this queue to work on.
  std::queue<Graph> to_be_processed;
  // The results of the final enumeration step.
  // Values: 3-tuple (
  //    id of the graph,
  //    the base graph,
  //    the graph with the minimum ratio among all graphs generated from the base graph).
  std::vector<std::tuple<int, Graph, Graph>> results;
  // The number of graphs that have been dequeued from the to_be_processed queue.
  int to_be_processed_id;

 protected:
  // Returns the ratio of the given graph.
  // The subclass must override this function, to implement which ratio to minimize in the search
  // which may be theta_ratio, zeta_ratio, etc.
  virtual Fraction get_ratio(const Graph& g) const = 0;

  // Returns true if g contains a forbidden subgraph, which has v as a vertex.
  // The subclass must override this function to implement which subgraph to forbid.
  virtual bool contains_forbidden_subgraph(Graph& g, int v) const = 0;

 public:
  // Constructs the Grower object.
  // log_stream is used for status reporting and debugging purpose.
  Grower(int num_worker_threads_, bool skip_final_enum_, bool use_min_theta_opt_,
         bool use_contains_Tk_opt_, int start_idx_, int end_idx_, bool search_ratio_graph_ = false,
         Fraction ratio_to_search_ = Fraction(1, 1));

  // Sets the logging streams.
  // If `details_per_graph` is true, print the min ratio value from each base graph into
  // the details log.
  void set_logging(std::ostream* summary, std::ostream* detail, std::ostream* result);

  // Finds all canonical isomorphism class representations with up to max_n vertices.
  void grow();

  // Returns the growth results.
  const std::vector<std::tuple<int, Graph, Graph>>& get_results() const { return results; }

  // For debugging and testing purpose: override the number of generations and seconds
  // between printing stats.
  void set_stats_print_interval(uint64 check_every_n_gen, int print_every_n_seconds);
};