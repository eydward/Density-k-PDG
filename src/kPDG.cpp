#include "graph.h"
#include "graph.hpp"
#include "grower.h"

int main() {
  Counters::initialize();

  Grower<3, 4> s;
  s.grow();
  s.print(true);

  Counters::print_counters();
  return 0;
}
