#include "counters.h"
#include "graph.h"
#include "grower.h"

void print_usage() {
  std::cout << "Usage: kPDG K N T [start_idx, end_idx]\n"
            << "  Each argument is an integer, K and N are required, others optional.\n"
            << "  K = Number of vertices in each edge.\n"
            << "  N = Total number of vertices in a graph.  2 <= K <= N <= 7.\n"
            << "  T = Number of worker threads. (0 means don't use threads).\n"
            << "  (optional) start_idx and end_index: the range of graph indices in the final\n"
            << "    enumeration phase (inclusive on both ends), allowing the search to run on\n"
            << "    multiple computers independently. They must be both omitted or both included\n"
            << "    in the arguments. end_idx can be larger than number of graphs available.\n";
}

// Returns current time in YYYYMMDD-HHmmss format. Used in the log file name.
std::string get_current_time() {
  std::time_t current_time = std::time(0);
  const std::tm* now = std::localtime(&current_time);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", now);
  return buf;
}

int main(int argc, char* argv[]) {
  if (argc != 4 && argc != 6) {
    print_usage();
    return -1;
  }
  int k = atoi(argv[1]);
  int n = atoi(argv[2]);
  int t = atoi(argv[3]);
  int start_idx = 0, end_idx = 0;
  if (argc == 6) {
    start_idx = atoi(argv[4]);
    end_idx = atoi(argv[5]);
  }

  if (k < 2 || n > 7 || k > n || t < 0 || t > 128 || start_idx < 0 || end_idx < start_idx) {
    std::cout << "Invalid command line arguments. See usage for details.\n";
    print_usage();
    return -1;
  }

  Graph::set_global_graph_info(k, n);

  std::string log_file_name = "kPDG_v8_" + std::to_string(Graph::K) + "_" +
                              std::to_string(Graph::N) + "_" + std::to_string(t) + "_" +
                              std::to_string(start_idx) + "_" + std::to_string(end_idx) + "_" +
                              get_current_time();
  std::string arguments =
      "kPDG run arguments: K=" + std::to_string(Graph::K) + ", N=" + std::to_string(Graph::N) +
      ", TOTAL_EDGES=" + std::to_string(Graph::TOTAL_EDGES) + ", THREADS=" + std::to_string(t) +
      ", idx_range=[" + std::to_string(start_idx) + ", " + std::to_string(end_idx) + "]\n\n";
  std::cout << "Log file path: " << log_file_name + ".log, " << log_file_name + "_detail.log"
            << "\n";
  std::ofstream log(log_file_name + ".log");
  std::ofstream detail_log(log_file_name + "_detail.log");
  std::cout << arguments;
  log << arguments;

  Counters::initialize(&log);

  Grower s(t, start_idx, end_idx, &log, &detail_log);
  s.grow();

  std::cout << "\nALL DONE. Final result:\n";
  log << "ALL DONE. Final result:\n";
  Counters::print_counters();
  log.flush();
  log.close();
  detail_log.flush();
  detail_log.close();
  return 0;
}
