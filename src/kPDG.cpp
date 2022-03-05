#include "graph.h"
#include "graph.hpp"
#include "grower.h"

int main() {
  Counters::initialize();

  Grower<6, 8> s;
  s.grow();
  s.print(false);

  Counters::print_counters();
  return 0;
}
