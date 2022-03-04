#include "graph.h"
#include "graph.hpp"
#include "grower.h"

int main() {
  Counters::initialize();

  Grower<3, 7> s;
  s.grow();
  s.print(false);

  Counters::print_counters();
  return 0;
}
