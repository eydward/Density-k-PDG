#pragma once
#include <bits/stdc++.h>

#include "fraction.h"
#include "graph.h"

// Holds all statistical counters to keep track of number of operations during the search.
class Counters {
 private:
  // The smallest ratio value observed so far (depending on the computational goal, it may be
  // theta_ratio, zeta_ratio, or some other quantity).
  static Fraction min_ratio;
  // The graph that produces the min_ratio.
  static Graph min_ratio_graph;
  // The start time of the computation.
  static std::chrono::time_point<std::chrono::steady_clock> start_time;
  // The time of the last status print.
  static std::chrono::time_point<std::chrono::steady_clock> last_print_time;
  // True if we are already in the final step to enumerate graphs.
  static bool in_final_step;

  static uint64 compute_vertex_signatures;
  static std::atomic<uint64> graph_copies;
  static std::atomic<uint64> graph_contains_Tk_tests;
  static uint64 growth_processed_graphs_in_current_step;
  static uint64 graph_accumulated_canonicals;
  static uint64 graph_canonicalize_ops;
  static uint64 graph_isomorphic_tests;
  // Number of isomorphic tests that returned true.
  static uint64 graph_isomorphic_true;
  // Number of isomorphic tests that have to use vertex permutations.
  static uint64 graph_isomorphic_expensive;
  // Number of isomorphic tests that result in no, but had identical hash
  static uint64 graph_isomorphic_hash_no;
  // Among the cases where hash code match but the graphs are not isomorphic, the number of cases
  // where the codegree info are different.
  static uint64 graph_isomorphic_codeg_diff;
  static uint64 graph_identical_tests;
  static uint64 graph_permute_ops;
  static uint64 graph_permute_canonical_ops;
  static uint64 growth_vertex_count;
  static uint64 growth_total_graphs_in_current_step;
  static uint64 growth_accumulated_canonicals_in_current_step;
  static uint64 growth_num_base_graphs_in_final_step;
  static uint64 edgegen_tk_skip;           // How many notify_contain_tk_skip().
  static uint64 edgegen_tk_skip_bits;      // How many bits did notify_contain_tk_skip() skip.
  static uint64 edgegen_theta_edges_skip;  // How many skips due to min_theta opt, not enough edges.
  static uint64 edgegen_theta_directed_edges_skip;  // How many skips min_theta opt, directed.
  static uint64 edgegen_edge_sets;  // How many edge sets returned from this generator.

  // How many graphs found to be <= given ratio value, in ratio-graph search.
  static uint64 ratio_graph_count;

  // Prints the counter values the given output stream.
  static void print_counters_to_stream(std::ostream& os);

 public:
  // The summary/detail/result log files, can be nullptr.
  static std::ostream* log;
  static std::ostream* log_detail;
  static std::ostream* log_result;

  // Constructs the log file names using the given the parameters, and creates the log files.
  static void initialize_logging(const std::string& prefix, int start_idx, int end_idx, int threads,
                                 bool search_ratio_graphs, Fraction search_ratio);
  // Flushes and closes the log files.
  static void close_logging();

  // Resets all values to 0, and starts the stopwatch, which will be used
  // by print_counters to calculate elapsed time.
  static void initialize();

  static Fraction get_min_ratio() { return min_ratio; }
  static const Graph& get_min_ratio_graph() { return min_ratio_graph; }
  static uint64 get_ratio_graph_count() { return ratio_graph_count; }
  static void increment_compute_vertex_signatures() { ++compute_vertex_signatures; }
  static void increment_graph_copies() { ++graph_copies; }
  static void increment_graph_canonicalize_ops() { ++graph_canonicalize_ops; }
  static void increment_graph_isomorphic_tests() { ++graph_isomorphic_tests; }
  static void increment_graph_isomorphic_true() { ++graph_isomorphic_true; }
  static void increment_graph_isomorphic_expensive() { ++graph_isomorphic_expensive; }
  static void increment_graph_isomorphic_hash_no() { ++graph_isomorphic_hash_no; }
  static void increment_graph_isomorphic_codeg_diff() { ++graph_isomorphic_codeg_diff; }
  static void increment_graph_identical_tests() { ++graph_identical_tests; }
  static void increment_graph_permute_ops() { ++graph_permute_ops; }
  static void increment_graph_permute_canonical_ops() { ++graph_permute_canonical_ops; }
  static void increment_graph_contains_Tk_tests() { ++graph_contains_Tk_tests; }
  static void increment_growth_processed_graphs_in_current_step() {
    ++growth_processed_graphs_in_current_step;
  }
  // Initialize the counters before starting the ratio-graph search.
  static void initialize_ratio_graph_search(Fraction ratio_threshold);
  // Notify that a graph matching the ratio value is found during ratio-graph search.
  static void notify_ratio_graph_found(const Graph& g, Fraction ratio);

  // Starting a new step in growth.
  static void new_growth_step(uint64 vertex_count, uint64 total_graphs_in_current_step);

  // Starting the final enumeration phase.
  static void enter_final_step(uint64 num_base_graphs);

  // If the given graph's ratio value is less than min_ratio, assign it to min_ratio.
  static void observe_ratio(const Graph& g, Fraction ratio, uint64 graphs_processed = 1);

  // Adds the edge gen stats to the counters.
  static void observe_edgegen_stats(uint64 tk_skip, uint64 tk_skip_bits, uint64 theta_edges_skip,
                                    uint64 theta_directed_edges_skip, uint64 edge_sets);

  // Print status if sufficient time has elapsed since the last print.
  static void print_at_time_interval();

  // Prints the "all done" message to console and summary log.
  static void print_done_message();

  // Prints the counter values to console and log file.
  static void print_counters();

  // Returns current time in YYYYMMDD-HHmmss format. Used in the log file name.
  static std::string get_current_time();
};
