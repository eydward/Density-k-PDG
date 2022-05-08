#include "counters.h"

#include <bits/stdc++.h>

constexpr int PRINT_EVERY_N_SECONDS = 100;

Fraction Counters::min_ratio = Fraction::infinity();
Graph Counters::min_ratio_graph{};
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
uint64 Counters::growth_vertex_count = 0;
uint64 Counters::growth_total_graphs_in_current_step = 0;
uint64 Counters::growth_accumulated_canonicals_in_current_step = 0;
uint64 Counters::growth_num_base_graphs_in_final_step = 0;
uint64 Counters::edgegen_tk_skip = 0;
uint64 Counters::edgegen_tk_skip_bits = 0;
uint64 Counters::edgegen_theta_edges_skip = 0;
uint64 Counters::edgegen_theta_directed_edges_skip = 0;
uint64 Counters::edgegen_edge_sets = 0;
uint64 Counters::ratio_graph_count = 0;
bool Counters::in_final_step = false;

std::ostream* Counters::log = nullptr;
std::ostream* Counters::log_detail = nullptr;
std::ostream* Counters::log_result = nullptr;

// The version number that appears in the log file names and summary log.
constexpr auto VERSION = "V12";

// Constructs the log file names using the given the parameters, and creates the log files.
void Counters::initialize_logging(const std::string& prefix, int start_idx, int end_idx,
                                  int threads, bool search_ratio_graphs, Fraction search_ratio,
                                  bool use_detail_log) {
  std::string log_file_name = prefix + "_" + VERSION + "_K" + std::to_string(Graph::K) + "_N" +
                              std::to_string(Graph::N) + "_" + std::to_string(start_idx) + "_" +
                              std::to_string(end_idx) + "_T" + std::to_string(threads) + "_" +
                              get_current_time();
  std::string arguments =
      prefix + " " + VERSION + " run arguments: K=" + std::to_string(Graph::K) +
      ", N=" + std::to_string(Graph::N) + ", TOTAL_EDGES=" + std::to_string(Graph::TOTAL_EDGES) +
      ", THREADS=" + std::to_string(threads) + ", idx_range=[" + std::to_string(start_idx) + ", " +
      std::to_string(end_idx) + "]\n" +
      (search_ratio_graphs ? "Searching graphs with min_ratio " + search_ratio.to_string() + "\n"
                           : std::string("")) +
      "\n";
  std::cout << "Log file path: " << log_file_name + ".log, ";
  log = new std::ofstream(log_file_name + ".log");
  if (use_detail_log) {
    std::cout << log_file_name + "_detail.log, ";
    log_detail = new std::ofstream(log_file_name + "_detail.log");
  }
  std::cout << log_file_name + "_result.log\n";
  log_result = new std::ofstream(log_file_name + "_result.log");
  std::cout << arguments;
  *log << arguments;

  initialize();
}

// Flushes and closes the log files.
void Counters::close_logging() {
  auto close_one = [](std::ostream* file) {
    if (file != nullptr) {
      file->flush();
      delete file;
    }
  };
  close_one(log);
  log = nullptr;
  close_one(log_detail);
  log_detail = nullptr;
  close_one(log_result);
  log_result = nullptr;
}

void Counters::initialize() {
  min_ratio = Fraction::infinity();
  last_print_time = start_time = std::chrono::steady_clock::now();
}

// If the given graph's ratio value is less than min_ratio, assign it to min_ratio.
void Counters::observe_ratio(const Graph& g, Fraction ratio, uint64 graphs_processed) {
  graph_accumulated_canonicals += graphs_processed;
  growth_accumulated_canonicals_in_current_step += graphs_processed;
  if (ratio < min_ratio) {
    min_ratio = ratio;
    min_ratio_graph = g;
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
  min_ratio = Fraction::infinity();
  growth_num_base_graphs_in_final_step = num_base_graphs;
  growth_processed_graphs_in_current_step = 0;
}

void Counters::initialize_ratio_graph_search(Fraction ratio_threshold) {
  min_ratio = ratio_threshold;
  ratio_graph_count = 0;
}
void Counters::notify_ratio_graph_found(const Graph& g, Fraction ratio) {
  ++ratio_graph_count;
  if (ratio <= min_ratio) {
    min_ratio = ratio;
    min_ratio_graph = g;
  }
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

// Returns current time in YYYYMMDD-HHmmss format. Used in the log file name.
std::string Counters::get_current_time() {
  std::time_t current_time = std::time(0);
  const std::tm* now = std::localtime(&current_time);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", now);
  return buf;
}

// Helper function: print large numbers in a more readable format with a ` at the 10^6 positions.
std::string Counters::fmt(uint64 value) {
  constexpr uint64 M = 1000000;
  if (value < M) {
    return std::to_string(value);
  } else {
    std::string tail = std::to_string(value % M);
    tail = std::string(6 - std::min(6, static_cast<int>(tail.length())), '0') + tail;
    if (value < M * M) {
      return std::to_string(value / M) + "`" + tail;
    } else {
      std::string mid = std::to_string((value / M) % M);
      mid = std::string(6 - std::min(6, static_cast<int>(mid.length())), '0') + mid;
      return std::to_string(value / (M * M)) + "`" + mid + "`" + tail;
    }
  }
}
void Counters::print_counters_to_stream(std::ostream& os) {
  const auto end = std::chrono::steady_clock::now();
  os << "\n--------Wall clock time:  "
     << fmt(std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count()) << "ms"
     << "\n    Current minimum ratio = " << min_ratio.to_string() << "\n    Produced by graph: ";
  min_ratio_graph.print_concise(os, false);

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
       << "\n    Isomorphic tests (total, true, expensive, false w/ =hash, identical, "
          "codeg_diff)= "
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
// Prints the "all done" message to console and summary log.
void Counters::print_done_message() {
  constexpr auto done_msg = "\n\n***************************\nALL DONE. Final result:\n";
  std::cout << done_msg;
  if (log != nullptr) {
    *log << done_msg;
  }
  print_counters();
}
