#include "graph.h"
#include "grower.h"

int main() {
  cout << sizeof(Graph<2, 7, 21>) << ", " << sizeof(Graph<3, 5, 10>) << ", "
       << sizeof(Graph<3, 7, 20>) << "\n";

  vector<uint8> vertices{8, 7, 8, 9, 3, 2, 1};
  vector<vector<uint8>> perm_sets;
  for (int v = 0; v < vertices.size() - 1; v++) {
    if (vertices[v + 1] == vertices[v]) {
      perm_sets.push_back(vector<uint8>());
      int t = v;
      while (vertices[t] == vertices[v]) {
        perm_sets.back().push_back(t);
        t++;
      }
      v = t;
    }
  }

  for (const auto& v : perm_sets) {
    cout << "*** ";
    for (uint8 i : v) {
      cout << (int)i << ", ";
    }
    cout << "\n";
  }

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

  // Grower<2, 7, 21> s;
  // s.grow(4);
  // s.print();

  return 0;
}
