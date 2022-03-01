#include "graph.h"
#include "grower.h"

int main() {
  cout << sizeof(Graph<2, 7, 21>) << ", " << sizeof(Graph<3, 5, 10>) << ", "
       << sizeof(Graph<3, 7, 20>) << "\n";

  // Graph<3, 5, 5> g;
  // g.add_edge(0b1011, UNDIRECTED);  // 013
  // g.add_edge(0b1110, 2);           // 123>2
  // g.add_edge(0b1101, UNDIRECTED);  // 023
  // g.add_edge(0b11100, 2);          // 234>2
  // g.init();
  // g.print();
  // g.canonicalize();
  // g.print();

  // Graph<3, 8, 5> h, f, e;
  // g.copy_without_init(h);
  // int p[8]{3, 4, 5, 6, 7, 0, 1, 2};
  // h.permute(p, f);
  // f.print();
  // f.canonicalize(e);
  // e.print();

  Grower<2, 7, 21> s;
  s.grow(4);
  s.print();

  return 0;
}
