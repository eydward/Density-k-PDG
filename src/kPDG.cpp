#include "counters.h"
#include "graph.h"
#include "grower.h"

void print_usage() {
  std::cout << "Usage: kPDG K N [print_graphs]\n"
            << "  Each argument is an integer, K and N are required, others optional.\n"
            << "  K = Number of vertices in each edge.\n"
            << "  N = Total number of vertices in a graph.  2 <= K <= N <= 7.\n"
            << "  (optional) print_graphs: 0 (default) or 1. 1 = print the accumulated graphs.\n";
}

// Returns current time in YYYYMMDD-HHmmss format.
std::string get_current_time() {
  std::time_t current_time = std::time(0);
  const std::tm* now = std::localtime(&current_time);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", now);
  return buf;
}

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

  // Get current time, to be used in the log file path.
  std::string log_path = "kPDG_v6run_" + std::to_string(Graph::K) + "_" + std::to_string(Graph::N) +
                         "_" + get_current_time() + ".log";
  std::string arguments = "kPDG run arguments: K=" + std::to_string(Graph::K) +
                          ", N=" + std::to_string(Graph::N) +
                          ", TOTAL_EDGES=" + std::to_string(Graph::TOTAL_EDGES) +
                          ", print_graph=" + std::to_string(print_graph) +
                          "\n  sizeof(Edge)=" + std::to_string(sizeof(Edge)) +
                          ", sizeof(VertexSignature)=" + std::to_string(sizeof(VertexSignature)) +
                          ", sizeof(Graph)=" + std::to_string(sizeof(Graph)) + "\n\n";
  std::cout << "Log file path: " << log_path << "\n";
  std::ofstream log(log_path);
  std::cout << arguments;
  log << arguments;

  Counters::initialize(&log);

  Grower s(print_graph, &log);
  s.grow();

  std::cout << "\nALL DONE. Final result:\n";
  log << "ALL DONE. Final result:\n";
  Counters::print_counters();
  log.flush();
  log.close();
  return 0;
}
