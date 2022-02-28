#include "graph.h"

int main() {
  cout << sizeof(Graph<3, 5, 10>) << ", " << sizeof(Graph<3, 7, 20>) << "\n";

  Graph<3, 5, 10> g;
  g.add_edge(0b11100, UNDIRECTED);
  g.add_edge(0b1100010, 5);
  g.add_edge(0b1110, 2);
  g.add_edge(0b1011, UNDIRECTED);
  g.init();
  g.print();
  return 0;
}
