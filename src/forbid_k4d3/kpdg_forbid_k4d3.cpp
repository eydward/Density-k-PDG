#include "../counters.h"
#include "graph_k4d3.h"

void print_usage() {
  std::cout << "Usage: kPDG-forbid-k4d3 N T\n"
            << "  Each argument is an integer\n"
            << "  N = Total number of vertices in a graph.  2 <= N <= 12.\n"
            << "  T = Number of worker threads. (0 means don't use threads,\n"
            << "                                 -1 means no final phase).\n";
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
  Counters::initialize_logging("kPDG-forbid-k4d3", 0, 0, t, false, Fraction(0, 1), false);
  GrowerK4D3 s(t, skip_final_enum, false, false, 0, 0);
  s.set_logging(Counters::log, Counters::log_detail, Counters::log_result);
  s.grow();
  Counters::print_done_message();
  Counters::close_logging();
  return 0;
}
