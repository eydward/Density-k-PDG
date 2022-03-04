#include "graph.h"
#include "graph.hpp"
#include "grower.h"

int main() {
  Counters::start_stopwatch();

  Grower<2, 4, 6> s;
  s.grow(4);
  s.print(true);

  Counters::print_counters();
  return 0;
}
