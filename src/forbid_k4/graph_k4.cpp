#include "../graph.h"

// Returns true if the given graph g contains F = {01, 02, 03, 12, 13, 23>3},
// namely a complete graph with 4 vertices and at least one directed edge, and the given
// vertex v is in F. Only works for 2-PDGs (K=3).
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing directed edges to undirected by forgetting directions.
bool contains_K4(const Graph& g, int v) {
  assert(Graph::K == 2);  // This logic only works for 2-PDG

  int neighbor_count = 0;                 // The number of neighbors of vertex v.
  int neighbors[MAX_VERTICES];            // The neighboring vertices of v.
  bool neighbors_directed[MAX_VERTICES];  // Whether the neighbor is from a directed edge.

  // First find all neighbors of v (ignore edge direction).
  for (uint8 e = 0; e < g.edge_count; e++) {
    if ((g.edges[e].vertex_set & (1 << v)) != 0) {
      // The edge contains the given vertex v. Find the other vertex u.
      int u = __builtin_ctz(static_cast<uint32>(g.edges[e].vertex_set & ~(1 << v)));
      neighbors[neighbor_count] = u;
      neighbors_directed[neighbor_count] = g.edges[e].head_vertex != UNDIRECTED;
      ++neighbor_count;
    }
  }

  // Next find the set of 3 neighbors, that are all linked by edges to each other:
  // For all 0<= i<j<k < neighbor_count_of_v, if the 3 edges ij, jk, ki are all in the
  // graph, and one of the 6 edges is directed, then the graph contains K4.
  for (int i = 0; i < neighbor_count; i++) {
    for (int j = i + 1; j < neighbor_count; j++) {
      for (int k = j + 1; k < neighbor_count; k++) {
        uint16 e_ij = (1 << neighbors[i]) | (1 << neighbors[j]);
        uint16 e_jk = (1 << neighbors[j]) | (1 << neighbors[k]);
        uint16 e_ki = (1 << neighbors[k]) | (1 << neighbors[i]);
        bool has_ij = false, has_jk = false, has_ki = false;
        bool directed_ij = false, directed_jk = false, directed_ki = false;
        for (uint16 e = 0; e < g.edge_count; e++) {
          if (g.edges[e].vertex_set == e_ij) {
            has_ij = true;
            directed_ij = g.edges[e].head_vertex != UNDIRECTED;
          } else if (g.edges[e].vertex_set == e_jk) {
            has_jk = true;
            directed_jk = g.edges[e].head_vertex != UNDIRECTED;
          } else if (g.edges[e].vertex_set == e_ki) {
            has_ki = true;
            directed_ki = g.edges[e].head_vertex != UNDIRECTED;
          }
        }
        if (has_ij && has_jk && has_ki &&
            (directed_ij || directed_jk || directed_ki || neighbors_directed[i] ||
             neighbors_directed[j] || neighbors_directed[k])) {
          return true;
        }
      }
    }
  }
  // If it gets here, we know the graph doesn't contain K4.
  return false;
}
