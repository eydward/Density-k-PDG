#include "graph.h"
#include "graph.hpp"
#include "grower.h"
#include "grower.hpp"

void print_usage() {
  std::cout << "Usage: kPDG K N [print_graphs]\n"
            << "  Each argument is an integer, K and N are required, others optional.\n"
            << "  K = Number of vertices in each edge.\n"
            << "  N = Total number of vertices in a graph.  2 <= K <= N <= 8.\n"
            << "  (optional) print_graphs: 0 (default) or 1. 1 = print the accumulated graphs.\n";
}

template <int K, int N>
void run(std::ostream* log, bool print_graph) {
  Grower<K, N> s(log);
  s.grow();
  s.print(print_graph);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    print_usage();
    return -1;
  }
  int K = atoi(argv[1]);
  int N = atoi(argv[2]);
  bool print_graph = false;
  if (argc >= 4) {
    print_graph = atoi(argv[3]) != 0;
  }
  if (K < 2 || N > 8 || K > N) {
    std::cout << "Invalid command line arguments. See usage for details.\n";
    print_usage();
    return -1;
  }

  std::string log_path = "kPDG_run_" + std::to_string(K) + "_" + std::to_string(N) + ".log";
  std::string arguments = "kPDG run arguments: K=" + std::to_string(K) +
                          ", N=" + std::to_string(N) +
                          ", print_graph=" + std::to_string(print_graph) + "\n\n";
  std::cout << "Log file path: " << log_path << "\n";
  std::ofstream log(log_path);
  std::cout << arguments;
  log << arguments;

  Counters::initialize(&log);

#define RUN(k, n)                 \
  if (K == k && N == n) {         \
    run<k, n>(&log, print_graph); \
    break;                        \
  }

  // This code block is a bit unfortunate. In order to minimize dynamic heap allocation,
  // We use C++ templates extensively. Which means K and N has to be compile-time known,
  // so we have to do this hack in order to accept command line arguments.
  do {
    RUN(2, 2);
    RUN(2, 3);
    RUN(2, 4);
    RUN(2, 5);
    RUN(2, 6);
    RUN(2, 7);
    RUN(2, 8);
    RUN(3, 3);
    RUN(3, 4);
    RUN(3, 5);
    RUN(3, 6);
    RUN(3, 7);
    RUN(3, 8);
    RUN(4, 4);
    RUN(4, 5);
    RUN(4, 6);
    RUN(4, 7);
    RUN(4, 8);
    RUN(5, 5);
    RUN(5, 6);
    RUN(5, 7);
    RUN(5, 8);
    RUN(6, 6);
    RUN(6, 7);
    RUN(6, 8);
    RUN(7, 7);
    RUN(7, 8);
    RUN(8, 8);
  } while (false);

  Counters::print_counters();
  log.flush();
  log.close();
  return 0;
}
