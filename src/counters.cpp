#include "counters.h"

#include <bits/stdc++.h>

Fraction Counters::min_theta(1E8, 1);
Edge Counters::min_theta_edges[255]{};
int Counters::min_theta_edge_count = 0;
uint64 Counters::compute_vertex_signatures = 0;
uint64 Counters::graph_allocations = 0;
uint64 Counters::chunk_allocations = 0;
uint64 Counters::graph_copies = 0;
uint64 Counters::graph_accumulated_canonicals = 0;
uint64 Counters::graph_canonicalize_ops = 0;
uint64 Counters::graph_isomorphic_tests = 0;
uint64 Counters::graph_isomorphic_expensive = 0;
uint64 Counters::graph_isomorphic_hash_no = 0;
uint64 Counters::graph_identical_tests = 0;
uint64 Counters::graph_permute_ops = 0;
uint64 Counters::graph_permute_canonical_ops = 0;
uint64 Counters::graph_contains_Tk_tests = 0;
std::chrono::time_point<std::chrono::steady_clock> Counters::start_time;
std::chrono::time_point<std::chrono::steady_clock> Counters::last_print_time;
std::ofstream* Counters::log = nullptr;
uint64 Counters::set_bucket_count = 0;
uint64 Counters::set_max_bucket_count = 0;
float Counters::set_load_factor = 0;
float Counters::set_max_load_factor = 0;

void Counters::initialize(std::ofstream* log_stream) {
  min_theta = Fraction(1E8, 1);
  compute_vertex_signatures = 0;
  graph_copies = 0;
  graph_canonicalize_ops = 0;
  graph_isomorphic_tests = 0;
  graph_isomorphic_expensive = 0;
  graph_isomorphic_hash_no = 0;
  graph_identical_tests = 0;
  graph_permute_ops = 0;
  graph_permute_canonical_ops = 0;
  graph_contains_Tk_tests = 0;
  last_print_time = start_time = std::chrono::steady_clock::now();
  log = log_stream;
}

// If the given graph's theta is less than min_theta, assign it to min_theta.
void Counters::observe_theta(const Graph& g) {
  ++graph_accumulated_canonicals;
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

void Counters::current_set_stats(uint64 bucket_count, uint64 max_bucket_count, float load_factor,
                                 float max_load_factor) {
  set_bucket_count = bucket_count;
  set_max_bucket_count = max_bucket_count;
  set_load_factor = load_factor;
  set_max_load_factor = max_load_factor;
}

void Counters::print_at_time_interval() {
  const auto now = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::seconds>(now - last_print_time).count() >= 100) {
    print_counters();
    last_print_time = now;
  }
}

void Counters::print_counters() {
  print_counters_to_stream(std::cout);
  if (log != nullptr) {
    print_counters_to_stream(*log);
    log->flush();
  }
}

void Counters::print_counters_to_stream(std::ostream& os) {
  const auto end = std::chrono::steady_clock::now();

  os << "\n---------- k=" << Graph::K << ", n=" << Graph::N << "-------------------------------"
     << "\nAccumulated canonicals\t= " << graph_accumulated_canonicals
     << "\nMinimum theta = " << min_theta.n << " / " << min_theta.d << "\nProduced by graph: ";
  Edge::print_edges(os, min_theta_edge_count, min_theta_edges);

  os << "\nWall clock time:  "
     << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << "ms"
     << "\nGraph allocs\t\t= " << graph_allocations << "\nChunk allocs\t\t= " << chunk_allocations
     << "\nCompute Vertex Signatures\t= " << compute_vertex_signatures
     << "\nGraph copies\t\t= " << graph_copies
     << "\nGraph canonicalize ops\t= " << graph_canonicalize_ops
     << "\nGraph permute ops\t= " << graph_permute_ops
     << "\nGraph permute canonical\t= " << graph_permute_canonical_ops
     << "\nGraph isomorphic tests\t= " << graph_isomorphic_tests
     << "\n    Expensive tests\t= " << graph_isomorphic_expensive
     << "\n    False w/ hash match\t= " << graph_isomorphic_hash_no
     << "\nGraph identical tests\t= " << graph_identical_tests
     << "\nGraph contains T_k\t= " << graph_contains_Tk_tests
     << "\nunordered_set(buckets, max_buckets, loadf, max_loadf)= (" << set_bucket_count << ", "
     << set_max_bucket_count << ", " << set_load_factor << ", " << set_max_load_factor << ")"
     << "\n--------------------------------------------------\n";
}