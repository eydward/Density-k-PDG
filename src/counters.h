#pragma once
#include <chrono>

#include "fraction.h"
using uint64 = unsigned __int64;

// Holds all statistical counters to keep track of number of operations during the search.
struct Counters {
  static Fraction min_theta;
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
  static std::chrono::time_point<std::chrono::steady_clock> start_time;

  // If the given theta is less than min_theta, assign it to min_theta.
  static void observe_theta(const Fraction& theta);
  // Resets all values to 0, and starts the stopwatch, which will be used
  // by print_counters to calculate elapsed time.
  static void initialize();
  // Prints the counter values to console.
  static void print_counters();
};
