#include "graph_k4d0.h"

// Returns true if the given graph g contains the complete transitive K4,
//   F={01>1, 02>2, 03>3, 12>2, 13>3, 23>3},
// and the given vertex v is a vertex in F. Only works for 2-PDGs (K=2).
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing undirected edges to directed edges.
bool contains_K4D0(Graph& g, int v) {
  // Applicable to the entire implementation of this function:
  //
  // In F={01>1, 02>2, 03>3, 12>2, 13>3, 23>3}, we'll call 0 the apex, and 123 the base.
  // Given v, if v is a vertex of F, and F is a subgrah of g, then either v is apex,
  // or it's connected to the apex either via an undirected edge, or via a directed edge where
  // the apex is the tail.

  assert(Graph::K == 2);  // This logic only works for 2-PDG

  // We need the degree info on all vertices that may be the apex.
  // Note this mutates `g`, and is the only reason why we can't use a `const Graph&`.
  g.compute_vertex_signature();

  // The total degree must be at least 3
  if (g.vertices[v].degree_undirected + g.vertices[v].degree_head + g.vertices[v].degree_tail < 3)
    return false;

  // Function that returns true if there is a subgraph F with the given apex in g.
  //
  // Aesthetic choice: we use a lambda here instead of declaring a separate function, to avoid
  // adding another friend function to the Graph class.
  auto contains_K4D0_apex = [g, v](int apex) {
    // Basic qualification of being apex.
    if (g.vertices[apex].degree_undirected + g.vertices[apex].degree_tail < 3) return false;

    int neighbor_count = 0;       // The number of undirected or head neighbors of the apex.
    int neighbors[MAX_VERTICES];  // The undirected or head neighboring vertices of apex.

    // Find all undirected and head neighbors of apex.
    for (int e = 0; e < g.edge_count; e++) {
      if (g.edges[e].head_vertex != apex && (g.edges[e].vertex_set & (1 << apex)) != 0) {
        // The edge contains the given vertex apex. Find the other vertex u.
        int u = __builtin_ctz(static_cast<uint32>(g.edges[e].vertex_set & ~(1 << apex)));
        neighbors[neighbor_count] = u;
        ++neighbor_count;
      }
    }
    assert(neighbor_count >= 3);

    // Loop through all sets of 3 undirected and head neighbors of apex, try to find a good base.
    // The neighbor_mask is used as a bit mask representing indices in the neighbors array,
    // e.g. 0b00001101 means the 0-th, 2nd, and 3rd elements in the neighbors array.
    for (uint16 neighbor_mask = 0; neighbor_mask < (1 << neighbor_count); neighbor_mask++) {
      // Ignore unless we have a 3-vertex set.
      if (__builtin_popcount(neighbor_mask) != 3) continue;

      // First transform the neighbor index mask into vertex mask.
      // Example of vertex mask: 0b00001101 means vertices 0, 2, 3.
      uint16 vertex_mask = 0;
      uint32 nm = neighbor_mask;
      for (int i = 0; i < 3; i++) {
        int idx = __builtin_ctz(nm);
        vertex_mask |= 1 << neighbors[idx];
        nm &= ~(1 << idx);
      }
      assert(nm == 0);
      assert(__builtin_popcount(vertex_mask) == 3);
      // Ignore unless either the apex or the 3-vertex base contains v.
      if (apex != v && (vertex_mask & (1 << v)) == 0) continue;

      // Find all edges in the base. Since there are only 3 vertices, there are <=3 edges.
      int base_edge_indices[3]{0};
      int base_edge_count = 0;
      for (int e = 0; e < g.edge_count; e++) {
        if (__builtin_popcount(g.edges[e].vertex_set & vertex_mask) == 2) {
          base_edge_indices[base_edge_count++] = e;
        }
      }
      // Ignore if there are less than 3 edges in the base.
      if (base_edge_count < 3) continue;

      // Check if the base contains a transitive triangle as a subgraph.
      // Recall the subgraph definition in the summary comment of this function.
      //
      // If there is any undirected edge, we have a transitive base.
      if (g.edges[base_edge_indices[0]].head_vertex == UNDIRECTED ||
          g.edges[base_edge_indices[1]].head_vertex == UNDIRECTED ||
          g.edges[base_edge_indices[2]].head_vertex == UNDIRECTED) {
        return true;
      }
      // Now all 3 edges are directed, we have a transitive base as long as it's not a 3-cycle.
      if (g.edges[base_edge_indices[0]].head_vertex == g.edges[base_edge_indices[1]].head_vertex ||
          g.edges[base_edge_indices[1]].head_vertex == g.edges[base_edge_indices[2]].head_vertex ||
          g.edges[base_edge_indices[0]].head_vertex == g.edges[base_edge_indices[2]].head_vertex) {
        return true;
      }
    }
    // All search done, we didn't find a F with the given apex.
    return false;
  };

  // Back to the main body of the contains_K4D0 function. First check v as apex.
  if (contains_K4D0_apex(v)) return true;
  // Check all vertices u as apex, where u is connected to v by either an undirected edge,
  // or a directed edge in which v is the head.
  for (int e = 0; e < g.edge_count; e++) {
    if ((g.edges[e].head_vertex == UNDIRECTED || g.edges[e].head_vertex == v) &&
        (g.edges[e].vertex_set & (1 << v)) != 0) {
      // The edge contains the given vertex apex. Find the other vertex u.
      int u = __builtin_ctz(static_cast<uint32>(g.edges[e].vertex_set & ~(1 << v)));
      if (contains_K4D0_apex(u)) return true;
    }
  }

  return false;
}
