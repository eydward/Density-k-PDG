#include "counters.h"
#include "graph.h"
#include "grower.h"

void print_usage() {
  std::cout << "Usage: kPDG K N T [restart_idx]\n"
            << "  Each argument is an integer, K and N are required, others optional.\n"
            << "  K = Number of vertices in each edge.\n"
            << "  N = Total number of vertices in a graph.  2 <= K <= N <= 7.\n"
            << "  T = Number of worker threads. (0 means don't use threads).\n"
            << "  (optional) restart_idx = the starting index in the final enumeration phase,\n"
            << "             can be used to restart a partially completed run.\n";
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
  if (argc < 4) {
    print_usage();
    return -1;
  }
  int k = atoi(argv[1]);
  int n = atoi(argv[2]);
  int t = atoi(argv[3]);
  int restart_idx = (argc >= 5 ? atoi(argv[4]) : 0);

  if (k < 2 || n > 7 || k > n || t < 0 || t > 128 || restart_idx < 0) {
    std::cout << "Invalid command line arguments. See usage for details.\n";
    print_usage();
    return -1;
  }

  Graph::set_global_graph_info(k, n);

  std::string log_file_name = "kPDG_v6run_" + std::to_string(Graph::K) + "_" +
                              std::to_string(Graph::N) + "_" + std::to_string(t) + "_" +
                              std::to_string(restart_idx) + "_" + get_current_time();
  std::string arguments = "kPDG run arguments: K=" + std::to_string(Graph::K) +
                          ", N=" + std::to_string(Graph::N) +
                          ", TOTAL_EDGES=" + std::to_string(Graph::TOTAL_EDGES) +
                          ", THREADS=" + std::to_string(t) + "\n\n";
  std::cout << "Log file path: " << log_file_name + ".log, " << log_file_name + "_detail.log"
            << "\n";
  std::ofstream log(log_file_name + ".log");
  std::ofstream detail_log(log_file_name + "_detail.log");
  std::cout << arguments;
  log << arguments;

  Counters::initialize(&log);

  Grower s(t, restart_idx, &log, &detail_log);
  s.grow();

  std::cout << "\nALL DONE. Final result:\n";
  log << "ALL DONE. Final result:\n";
  Counters::print_counters();
  log.flush();
  log.close();
  return 0;
}
