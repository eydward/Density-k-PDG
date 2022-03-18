#pragma once
#include <bits/stdc++.h>

#include "fraction.h"
#include "graph.h"

// Holds all statistical counters to keep track of number of operations during the search.
class Counters {
 private:
  // The smallest theta value observed so far.
  static Fraction min_theta;
  // The graph that produces the min_theta.
  static Graph min_theta_graph;
  // The start time of the computation.
  static std::chrono::time_point<std::chrono::steady_clock> start_time;
  // The time of the last status print.
  static std::chrono::time_point<std::chrono::steady_clock> last_print_time;
  // The log file, can be nullptr.
  static std::ofstream* log;
  // True after the first status print.
  static bool has_printed;

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
  static uint64 growth_automorphisms_found;
  static uint64 growth_automorphisms_vset_skips;
  static uint64 growth_num_base_graphs_in_final_step;

  // Prints the counter values the given output stream.
  static void print_counters_to_stream(std::ostream& os);

 public:
  static Fraction get_min_theta() { return min_theta; }
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
  static uint64 increment_graph_contains_Tk_tests() { return ++graph_contains_Tk_tests; }
  static void increment_growth_processed_graphs_in_current_step() {
    ++growth_processed_graphs_in_current_step;
  }
  static void increment_growth_automorphisms_found() { ++growth_automorphisms_found; }
  static void increment_growth_automorphisms_vset_skips() { ++growth_automorphisms_vset_skips; }

  // Starting a new step in growth.
  static void new_growth_step(uint64 vertex_count, uint64 total_graphs_in_current_step);

  static void enter_final_step(uint64 num_base_graphs);

  // If the given graph's theta is less than min_theta, assign it to min_theta.
  static void observe_theta(const Graph& g, uint64 graphs_processed = 1);

  // Resets all values to 0, and starts the stopwatch, which will be used
  // by print_counters to calculate elapsed time.
  static void initialize(std::ofstream* log_stream = nullptr);

  // Print status if sufficient time has elapsed since the last print.
  static void print_at_time_interval();

  // Prints the counter values to console and log file.
  static void print_counters();
};
