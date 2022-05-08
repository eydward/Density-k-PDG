#include "counters.h"
#include "forbid_tk/graph_tk.h"
#include "graph.h"

void print_usage() {
  std::cout << "Usage: kPDG K N T [start_idx, end_idx, [theta_n, theta_d]]\n"
            << "  Each argument is an integer, K and N are required, others optional.\n"
            << "  K = Number of vertices in each edge.\n"
            << "  N = Total number of vertices in a graph.  2 <= K <= N <= 12.\n"
            << "  T = Number of worker threads. (0 means don't use threads,\n"
            << "                                 -1 means no final phase).\n"
            << "  (optional) start_idx and end_index: the range of graph indices in the final\n"
            << "    enumeration phase (inclusive on both ends), allowing the search to run on\n"
            << "    multiple computers independently. They must be both omitted or both included\n"
            << "    in the arguments. end_idx can be larger than number of graphs available.\n"
            << "  (optional) theta_n and theta_d: if specified, find all graphs that produce the\n"
            << "    given theta value and print into the result log, instead of searching for\n"
            << "    the minimum theta value.\n";
}

int main(int argc, char* argv[]) {
  if (argc != 4 && argc != 6 && argc != 8) {
    print_usage();
    return -1;
  }
  int k = atoi(argv[1]);
  int n = atoi(argv[2]);
  int t = atoi(argv[3]);
  int start_idx = 0, end_idx = 0;
  int theta_n = 0, theta_d = 1;
  bool search_theta_graphs = false;
  if (argc >= 6) {
    start_idx = atoi(argv[4]);
    end_idx = atoi(argv[5]);
  }
  if (argc == 8) {
    theta_n = atoi(argv[6]);
    theta_d = atoi(argv[7]);
    search_theta_graphs = true;
  }

  bool skip_final_enum = false;
  if (t < 0) {
    skip_final_enum = true;
    t = 0;
  }

  if (k < 2 || n > 12 || k > n || t < 0 || t > 128 || start_idx < 0 || end_idx < start_idx) {
    std::cout << "Invalid command line arguments. See usage for details.\n";
    print_usage();
    return -1;
  }
  if (compute_binom(n, k) > MAX_EDGES) {
    std::cout << "K=" << k << ", N=" << n
              << " requires edge count more than MAX_EDGES=" << MAX_EDGES << ". Cannot execute.\n";
    return -1;
  }

  Graph::set_global_graph_info(k, n);
  Counters::initialize_logging(search_theta_graphs ? "kPDG_thetagraph" : "kPDG", start_idx, end_idx,
                               t, search_theta_graphs, Fraction(theta_n, theta_d), true);
  GrowerTk s(t, skip_final_enum, true, true, start_idx, end_idx, search_theta_graphs,
             Fraction(theta_n, theta_d));
  s.set_logging(Counters::log, Counters::log_detail, Counters::log_result);
  s.grow();
  Counters::print_done_message();
  Counters::close_logging();
  return 0;
}
