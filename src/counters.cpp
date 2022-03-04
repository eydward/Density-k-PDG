#include "graph.h"

// The constant n choose k. Used to compute theta.
template <int K, int N, int MAX_EDGES>
constexpr int Graph<K, N, MAX_EDGES>::BINOM_NK;

Fraction Counters::min_theta(1E8, 1);
uint64 Counters::graph_inits = 0;
uint64 Counters::graph_copies = 0;
uint64 Counters::graph_canonicalize_ops = 0;
uint64 Counters::graph_isomorphic_tests = 0;
uint64 Counters::graph_isomorphic_expensive = 0;
uint64 Counters::graph_isomorphic_hash_no = 0;
uint64 Counters::graph_identical_tests = 0;
uint64 Counters::graph_permute_ops = 0;
uint64 Counters::graph_permute_canonical_ops = 0;
uint64 Counters::graph_contains_Tk_tests = 0;
std::chrono::time_point<std::chrono::steady_clock> Counters::start_time;

void Counters::observe_theta(const Fraction& theta) {
  if (theta < min_theta) {
    min_theta = theta;
  }
}

void Counters::start_stopwatch() { start_time = std::chrono::steady_clock::now(); }

void Counters::print_counters() {
  const auto end = std::chrono::steady_clock::now();

  cout << "\n--------------------------------------------------"
       << "\n-------- Statistical Counters---------------------"
       << "\nWall clock time:  "
       << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << "ms"
       << "\nMinimum theta\t\t= " << min_theta.n << " / " << min_theta.d
       << "\nGraph inits\t\t= " << graph_inits << "\nGraph copies\t\t= " << graph_copies
       << "\nGraph canonicalize ops\t= " << graph_canonicalize_ops
       << "\nGraph permute ops\t= " << graph_permute_ops
       << "\nGraph permute canonical\t= " << graph_permute_canonical_ops
       << "\nGraph isomorphic tests\t= " << graph_isomorphic_tests
       << "\n    Expensive tests\t= " << graph_isomorphic_expensive
       << "\n    False w/ hash match\t= " << graph_isomorphic_hash_no
       << "\nGraph identical tests\t= " << graph_identical_tests
       << "\nGraph contains T_k\t= " << graph_contains_Tk_tests;
  cout << "\n--------------------------------------------------\n";
}