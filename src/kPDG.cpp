#include "graph.h"

int main() {
  cout << sizeof(Graph<3, 5, 10>) << ", " << sizeof(Graph<3, 7, 20>) << "\n";

  Graph<3, 5, 5> g;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, 2);           // 123>2
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.add_edge(0b11100, 2);          // 234>2
  g.init();
  g.print();
  return 0;
}
