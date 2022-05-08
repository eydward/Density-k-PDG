#include "graph_tk.h"

#include "../counters.h"

// Returns true if the graph contains the generalized triangle T_k as a subgraph, where
// v is one of the vertices of the T_k subgraph.
// T_k is defined as (K+1)-vertex, 3-edge K-graph, with two undirected edges and one directed
// edge, where all edges share the same set of vertices except for {1,2,3}.
// For example T_2={12, 13, 23>3}, T_3={124, 134, 234>3}, T_4={1245, 1345, 2345>3}, etc.
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing directed edges to undirected by forgetting directions.
bool contains_Tk(const Graph& g, int v) {
  Counters::increment_graph_contains_Tk_tests();

  // There are two possibilities that $v \in T_k \subseteq H$.
  // (1) v is in the "triangle with stem cut off". Namely:
  //     exists vertices $x,y$, and vertex set $S$ with size $K-2$, $S$ disjoint
  //     from $v,x,y$, such that there are 3 edges: $S\cup vx, S\cup vy, S\cup xy$,
  //     and at least one of the 3 edges is directed, with the head being one of $v,x,y$.
  // (2) v is in the "common stem". Namely:
  //     exists vertices $x,y,z$, and vertex set $S$ with size $K-3$ and $x\in S$, $S$ disjoint
  //     from $x,y,z$, such that there are 3 edges: $S\cup xy, S\cup yz, S\cup zx$,
  //     and at least one of the 3 edges is directed, with the head being one of $x,y,z$.

  // Check for the two possibilities. For the most part, the checking logic is same.
  // Example to understand the code below:
  //                      possibility (1)         possibility (2)
  //                      S = {3,4,5,6}           S = {3,4,5,6}
  //                      K=6, v=2, x=1, y=0.     K=6, v=4, x=2, y=1, z=0.
  //   e_i  = 01111110    $S\cup vx$              $S\cup xy$
  //   e_j  = 01111101    $S\cup vy$              $S\cup yz$
  //   m    = 00000011
  //   mask = 01111111
  //   e_k  = 01111011    $S\cup xy$              $S\cup zx$
  //   stem = 01111000
  //   xyz  = 00000111
  for (int i = 0; i < g.edge_count - 1; i++) {
    uint16 e_i = g.edges[i].vertex_set;
    if ((e_i & (1 << v)) == 0) continue;

    for (int j = i + 1; j < g.edge_count; j++) {
      uint16 e_j = g.edges[j].vertex_set;
      if ((e_j & (1 << v)) == 0) continue;

      uint16 m = e_i ^ e_j;
      if (__builtin_popcount(m) == 2) {
        uint16 mask = m | g.edges[i].vertex_set;
        for (int k = 0; k < g.edge_count; k++) {
          if (k == i || k == j) continue;

          uint16 e_k = g.edges[k].vertex_set;
          if (__builtin_popcount(mask ^ e_k) == 1) {
            uint16 stem = m ^ e_k;
            uint16 xyz = (e_i | e_j | e_k) & ~stem;
            if ((g.edges[i].head_vertex != UNDIRECTED &&
                 (xyz & (1 << g.edges[i].head_vertex)) != 0) ||
                (g.edges[j].head_vertex != UNDIRECTED &&
                 (xyz & (1 << g.edges[j].head_vertex)) != 0) ||
                (g.edges[k].head_vertex != UNDIRECTED &&
                 (xyz & (1 << g.edges[k].head_vertex)) != 0)) {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}
