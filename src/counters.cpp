#include "counters.h"

#include <bits/stdc++.h>

constexpr int PRINT_EVERY_N_SECONDS = 100;

Fraction Counters::min_theta(1E8, 1);
Graph Counters::min_theta_graph{};
uint64 Counters::compute_vertex_signatures = 0;
std::atomic<uint64> Counters::graph_copies = 0;
std::atomic<uint64> Counters::graph_contains_Tk_tests = 0;
uint64 Counters::growth_processed_graphs_in_current_step = 0;
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
std::chrono::time_point<std::chrono::steady_clock> Counters::start_time;
std::chrono::time_point<std::chrono::steady_clock> Counters::last_print_time;
std::ofstream* Counters::log = nullptr;
uint64 Counters::growth_vertex_count = 0;
uint64 Counters::growth_total_graphs_in_current_step = 0;
uint64 Counters::growth_accumulated_canonicals_in_current_step = 0;
uint64 Counters::growth_num_base_graphs_in_final_step = 0;
uint64 Counters::edgegen_tk_skip = 0;
uint64 Counters::edgegen_tk_skip_bits = 0;
uint64 Counters::edgegen_theta_edges_skip = 0;
uint64 Counters::edgegen_theta_directed_edges_skip = 0;
uint64 Counters::edgegen_edge_sets = 0;

bool Counters::in_final_step = false;

void Counters::initialize(std::ofstream* log_stream) {
  min_theta = Fraction(1E8, 1);
  last_print_time = start_time = std::chrono::steady_clock::now();
  log = log_stream;
}

// If the given graph's theta is less than min_theta, assign it to min_theta.
void Counters::observe_theta(const Graph& g, uint64 graphs_processed) {
  graph_accumulated_canonicals += graphs_processed;
  growth_accumulated_canonicals_in_current_step += graphs_processed;
  Fraction theta = g.get_theta();
  if (theta < min_theta) {
    min_theta = theta;
    min_theta_graph = g;
  }
  print_at_time_interval();
}

void Counters::observe_edgegen_stats(uint64 tk_skip, uint64 tk_skip_bits, uint64 theta_edges_skip,
                                     uint64 theta_directed_edges_skip, uint64 edge_sets) {
  edgegen_tk_skip += tk_skip;
  edgegen_tk_skip_bits += tk_skip_bits;
  edgegen_theta_edges_skip += theta_edges_skip;
  edgegen_theta_directed_edges_skip += theta_directed_edges_skip;
  edgegen_edge_sets += edge_sets;
}

void Counters::new_growth_step(uint64 vertex_count, uint64 total_graphs_in_current_step) {
  growth_vertex_count = vertex_count;
  growth_total_graphs_in_current_step = total_graphs_in_current_step;
  growth_accumulated_canonicals_in_current_step = 0;
  growth_processed_graphs_in_current_step = 0;
}

void Counters::enter_final_step(uint64 num_base_graphs) {
  in_final_step = true;
  growth_num_base_graphs_in_final_step = num_base_graphs;
  growth_processed_graphs_in_current_step = 0;
}

void Counters::print_at_time_interval() {
  const auto now = std::chrono::steady_clock::now();
  int seconds = std::chrono::duration_cast<std::chrono::seconds>(now - last_print_time).count();
  if (seconds >= PRINT_EVERY_N_SECONDS) {
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

// Helper function: print large numbers in a more readable format with a ` at the 10^6 position.
std::string fmt(uint64 value) {
  constexpr uint64 M = 1000000;
  if (value < M) {
    return std::to_string(value);
  } else {
    std::string tail = std::to_string(value % M);
    return std::to_string(value / M) + "`" +
           std::string(6 - std::min(6, static_cast<int>(tail.length())), '0') + tail;
  }
}
void Counters::print_counters_to_stream(std::ostream& os) {
  const auto end = std::chrono::steady_clock::now();

  os << "\n--------Wall clock time:  "
     << fmt(std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count()) << "ms"
     << "\n    Current minimum theta = " << min_theta.n << " / " << min_theta.d
     << "\n    Produced by graph: ";
  min_theta_graph.print_concise(os, false);

  if (in_final_step) {
    os << "    Base graphs processed / total = " << fmt(growth_processed_graphs_in_current_step)
       << " / " << fmt(growth_num_base_graphs_in_final_step) << ". Ops (copies, T_k, free)= ("
       << fmt(graph_copies) << ", " << fmt(graph_contains_Tk_tests) << ", "
       << fmt(growth_accumulated_canonicals_in_current_step)
       << ")\n    EdgeGen stats (tk-skip, bits, theta_edge_skip, theta_dir_skip, sets)= ("
       << fmt(edgegen_tk_skip) << ", " << fmt(edgegen_tk_skip_bits) << ", "
       << fmt(edgegen_theta_edges_skip) << ", " << fmt(edgegen_theta_directed_edges_skip) << ", "
       << fmt(edgegen_edge_sets) << ")\n";
  } else {
    os << "    Accumulated canonicals\t= " << fmt(graph_accumulated_canonicals)
       << "\n    Ops (vertex sig, copies, canonicalize, permute, T_k)= ("
       << fmt(compute_vertex_signatures) << ", " << fmt(graph_copies) << ", "
       << fmt(graph_canonicalize_ops) << ", " << fmt(graph_permute_canonical_ops) << ", "
       << fmt(graph_contains_Tk_tests) << ")"
       << "\n    Isomorphic tests (total, true, expensive, false w/ =hash, identical, codeg_diff)= "
       << "\n                     (" << fmt(graph_isomorphic_tests) << ", "
       << fmt(graph_isomorphic_true) << ", " << fmt(graph_isomorphic_expensive) << ", "
       << fmt(graph_isomorphic_hash_no) << ", " << fmt(graph_identical_tests) << ", "
       << fmt(graph_isomorphic_codeg_diff) << ")"
       << "\n    Growth stats(vertices, total in step, processed in step, accumulated in step)= ("
       << fmt(growth_vertex_count) << ", " << fmt(growth_total_graphs_in_current_step) << ", "
       << fmt(growth_processed_graphs_in_current_step) << ", "
       << fmt(growth_accumulated_canonicals_in_current_step) << ")\n";
  }
}