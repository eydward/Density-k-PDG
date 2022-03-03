#include "graph.h"
#include "graph.hpp"
#include "grower.h"

int main() {
  Counters::start_stopwatch();

  Grower<2, 7, 21> s;
  s.grow(5);
  s.print();

  Counters::print_counters();
  return 0;
}
