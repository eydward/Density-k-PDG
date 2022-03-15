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
uint64 Counters::graph_isomorphic_true = 0;
uint64 Counters::graph_isomorphic_expensive = 0;
uint64 Counters::graph_isomorphic_hash_no = 0;
uint64 Counters::graph_isomorphic_codeg_diff = 0;
uint64 Counters::graph_identical_tests = 0;
uint64 Counters::graph_permute_ops = 0;
uint64 Counters::graph_permute_canonical_ops = 0;
uint64 Counters::graph_contains_Tk_tests = 0;
std::chrono::time_point<std::chrono::steady_clock> Counters::start_time;
std::chrono::time_point<std::chrono::steady_clock> Counters::last_print_time;
std::ofstream* Counters::log = nullptr;
bool Counters::has_printed = false;
uint64 Counters::set_bucket_count = 0;
float Counters::set_load_factor = 0;
float Counters::set_max_load_factor = 0;
uint64 Counters::growth_vertex_count = 0;
uint64 Counters::growth_total_graphs_in_current_step = 0;
uint64 Counters::growth_processed_graphs_in_current_step = 0;
uint64 Counters::growth_accumulated_canonicals_in_current_step = 0;
uint64 Counters::growth_automorphisms_found = 0;
uint64 Counters::growth_automorphisms_vset_skips = 0;

void Counters::initialize(std::ofstream* log_stream) {
  min_theta = Fraction(1E8, 1);
  min_theta_edge_count = 0;
  last_print_time = start_time = std::chrono::steady_clock::now();
  log = log_stream;
}

// If the given graph's theta is less than min_theta, assign it to min_theta.
void Counters::observe_theta(const Graph& g) {
  ++graph_accumulated_canonicals;
  ++growth_accumulated_canonicals_in_current_step;
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

void Counters::new_growth_step(uint64 vertex_count, uint64 total_graphs_in_current_step) {
  growth_vertex_count = vertex_count;
  growth_total_graphs_in_current_step = total_graphs_in_current_step;
  growth_accumulated_canonicals_in_current_step = 0;
  growth_processed_graphs_in_current_step = 0;
}

void Counters::current_set_stats(uint64 bucket_count, float load_factor, float max_load_factor) {
  set_bucket_count = bucket_count;
  set_load_factor = load_factor;
  set_max_load_factor = max_load_factor;
}

void Counters::print_at_time_interval() {
  const auto now = std::chrono::steady_clock::now();
  int seconds = std::chrono::duration_cast<std::chrono::seconds>(now - last_print_time).count();
  if (!has_printed && seconds >= 10) {
    print_counters();
    has_printed = true;
  } else if (seconds >= 100) {
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

  os << "\n--------Wall clock time:  "
     << std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count() << "ms"
     << "\nCurrent minimum theta = " << min_theta.n << " / " << min_theta.d
     << "\nProduced by graph: ";
  Edge::print_edges(os, min_theta_edge_count, min_theta_edges);

  os << "\nAccumulated canonicals\t= " << graph_accumulated_canonicals
     << "\nAllocations (chunks, graphs)= (" << chunk_allocations << ", " << graph_allocations << ")"
     << "\nOps (vertex sig, copies, canonicalize, permute, T_k)= (" << compute_vertex_signatures
     << ", " << graph_copies << ", " << graph_canonicalize_ops << ", "
     << graph_permute_canonical_ops << ", " << graph_contains_Tk_tests << ")"
     << "\nIsomorphic tests (total, true, expensive, false w/ =hash, identical, codeg_diff)= "
     << "\n                 (" << graph_isomorphic_tests << ", " << graph_isomorphic_true << ", "
     << graph_isomorphic_expensive << ", " << graph_isomorphic_hash_no << ", "
     << graph_identical_tests << ", " << graph_isomorphic_codeg_diff << ")"
     << "\nunordered_set(buckets, max_buckets, loadf, max_loadf)= (" << set_bucket_count << ", "
     << set_load_factor << ", " << set_max_load_factor << ")"
     << "\nGrowth stats(vertex count, total in step, processed in step, accumulated in step)= ("
     << growth_vertex_count << ", " << growth_total_graphs_in_current_step << ", "
     << growth_processed_graphs_in_current_step << ", "
     << growth_accumulated_canonicals_in_current_step << ")"
     << "\nGrowth automorphism stats(found, vset skips)= (" << growth_automorphisms_found << ", "
     << growth_automorphisms_vset_skips << ")\n";
}