#include "../counters.h"
#include "../graph.h"
#include "graph_k4.h"

void print_usage() {
  std::cout << "Usage: k4problem N T\n"
            << "  Each argument is an integer\n"
            << "  N = Total number of vertices in a graph.  2 <= N <= 8.\n"
            << "  T = Number of worker threads. (0 means don't use threads,\n"
            << "                                 -1 means no final phase).\n";
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
  if (argc != 3) {
    print_usage();
    return -1;
  }
  int n = atoi(argv[1]);
  int t = atoi(argv[2]);
  bool skip_final_enum = false;
  if (t < 0) {
    skip_final_enum = true;
    t = 0;
  }

  if (n > 12 || 2 > n || t < 0 || t > 128) {
    std::cout << "Invalid command line arguments. See usage for details.\n";
    print_usage();
    return -1;
  }

  Graph::set_global_graph_info(2, n);

  std::string log_file_name =
      "K4problem_" + std::to_string(Graph::N) + "_" + std::to_string(t) + "_" + get_current_time();
  std::string arguments = "K4problem run arguments: K=2, N=" + std::to_string(Graph::N) +
                          ", TOTAL_EDGES=" + std::to_string(Graph::TOTAL_EDGES) +
                          ", THREADS=" + std::to_string(t) + "]\n\n";
  std::cout << "Log file path: " << log_file_name + ".log, " << log_file_name + "_detail.log, "
            << log_file_name + "_result.log\n";
  std::ofstream log(log_file_name + ".log");
  std::ofstream detail_log(log_file_name + "_detail.log");
  std::ofstream result_log(log_file_name + "_result.log");
  std::cout << arguments;
  log << arguments;

  Counters::initialize(&log);
  GrowerK4 s(t, skip_final_enum, false, false, 0, 0);
  s.set_logging(&log, &detail_log, &result_log);
  s.grow();
  const std::string done_msg = "\n\n***************************\nALL DONE. Final result:\n";
  std::cout << done_msg;
  log << done_msg;
  Counters::print_counters();

  log.flush();
  log.close();
  detail_log.flush();
  detail_log.close();
  result_log.close();
  return 0;
}
