#include "graph.h"
#include "graph.hpp"
#include "grower.h"
#include "grower.hpp"

void print_usage() {
  std::cout << "Usage: kPDG K N [print_graphs]\n"
            << "  Each argument is an integer, K and N are required, others optional.\n"
            << "  K = Number of vertices in each edge.\n"
            << "  N = Total number of vertices in a graph.  2 <= K <= N <= 7.\n"
            << "  (optional) print_graphs: 0 (default) or 1. 1 = print the accumulated graphs.\n";
}

template <int K, int N>
void run(std::ostream* log, bool print_graph) {}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    print_usage();
    return -1;
  }
  int k = atoi(argv[1]);
  int n = atoi(argv[2]);
  bool print_graph = false;
  if (argc >= 4) {
    print_graph = atoi(argv[3]) != 0;
  }
  if (k < 2 || n > 7 || k > n) {
    std::cout << "Invalid command line arguments. See usage for details.\n";
    print_usage();
    return -1;
  }

  Graph::set_global_graph_info(k, n);

  std::string log_path =
      "kPDG_run_" + std::to_string(Graph::K) + "_" + std::to_string(Graph::N) + ".log";
  std::string arguments = "kPDG run arguments: K=" + std::to_string(Graph::K) +
                          ", N=" + std::to_string(Graph::N) +
                          ", TOTAL_EDGES=" + std::to_string(Graph::TOTAL_EDGES) +
                          ", print_graph=" + std::to_string(print_graph) + "\n\n";
  std::cout << "Log file path: " << log_path << "\n";
  std::ofstream log(log_path);
  std::cout << arguments;
  log << arguments;

  Counters::initialize(&log);

  Grower s(&log);
  s.grow();
  s.print(print_graph);

  Counters::print_counters();
  log.flush();
  log.close();
  return 0;
}
