#pragma once
#include <bits/stdc++.h>

#include "fraction.h"
#include "graph.h"

// Holds all statistical counters to keep track of number of operations during the search.
struct Counters {
 private:
  // The smallest theta value observed so far.
  static Fraction min_theta;
  // The edges in the graph that produces the min_theta.
  static Edge min_theta_edges[255];
  // Number of edges in the graph that produces the min_theta.
  static int min_theta_edge_count;
  // Number of vertices in each edge.
  static int k;
  // Total number of vertices in the graph.
  static int n;
  // The start time of the computation.
  static std::chrono::time_point<std::chrono::steady_clock> start_time;
  // The time of the last status print.
  static std::chrono::time_point<std::chrono::steady_clock> last_print_time;
  // The log file, can be nullptr.
  static std::ofstream* log;

  static uint64 graph_inits;
  static uint64 graph_copies;
  static uint64 graph_canonicalize_ops;
  static uint64 graph_isomorphic_tests;
  // Number of isomorphic tests that have to use vertex permutations.
  static uint64 graph_isomorphic_expensive;
  // Number of isomorphic tests that result in no, but had identical hash
  static uint64 graph_isomorphic_hash_no;
  static uint64 graph_identical_tests;
  static uint64 graph_permute_ops;
  static uint64 graph_permute_canonical_ops;
  static uint64 graph_contains_Tk_tests;

 public:
  static Fraction get_min_theta() { return min_theta; }
  static void increment_graph_inits() { ++graph_inits; }
  static void increment_graph_copies() { ++graph_copies; }
  static void increment_graph_canonicalize_ops() { ++graph_canonicalize_ops; }
  static void increment_graph_isomorphic_tests() { ++graph_isomorphic_tests; }
  static void increment_graph_isomorphic_expensive() { ++graph_isomorphic_expensive; }
  static void increment_graph_isomorphic_hash_no() { ++graph_isomorphic_hash_no; }
  static void increment_graph_identical_tests() { ++graph_identical_tests; }
  static void increment_graph_permute_ops() { ++graph_permute_ops; }
  static void increment_graph_permute_canonical_ops() { ++graph_permute_canonical_ops; }
  static void increment_graph_contains_Tk_tests() { ++graph_contains_Tk_tests; }

  // If the given graph's theta is less than min_theta, assign it to min_theta.
  template <int K, int N>
  static void observe_theta(const Graph<K, N>& g) {
    k = K;
    n = N;
    Fraction theta = g.get_theta();
    if (theta < min_theta) {
      min_theta = theta;
      min_theta_edge_count = g.edge_count;
      for (int i = 0; i < g.edge_count; i++) {
        min_theta_edges[i] = g.edges[i];
      }
    }
    print_at_time_interval();
  }

  // Resets all values to 0, and starts the stopwatch, which will be used
  // by print_counters to calculate elapsed time.
  static void initialize(std::ofstream* log_stream = nullptr);
  // Prints the counter values to console and log file.
  static void print_counters();

 private:
  // Prints the counter values the given output stream.
  static void print_counters_to_stream(std::ostream& os);
  // Print status if sufficient time has elapsed since the last print.
  static void print_at_time_interval();
};
