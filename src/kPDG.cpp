#include "graph.h"
#include "grower.h"

int main() {
  Grower<2, 7, 21> s;
  s.grow(6);
  s.print();

  return 0;
}
