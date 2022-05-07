#include "../graph.h"

// Return true if the graphs contains {01,02,03,12>2,13>3,23>3}.
// Only works for 2-PDG. The given vertex v must be in the complete graph.
bool Graph::contains_K4D3(int v) {
  assert(K == 2);  // This logic only works for 2-PDG

  compute_vertex_signature();

  // In {01,02,03,12>2,13>3,23>3}, we'll call 0 the apex, and 123 the base.
  // Given v, either v is apex, or it's connected to the apex via an undirected edge.

  // Regardless v is apex or base, it must incident to at least one undirected edge.
  if (vertices[v].degree_undirected == 0) return false;
  // Also it's total degree must be at least 3
  if (vertices[v].degree_undirected + vertices[v].degree_head + vertices[v].degree_tail < 3)
    return false;

  // Function that returns true if there is a K4D3 with the given apex.
  auto contains_K4D3_apex = [this, v](int apex) {
    if (vertices[apex].degree_undirected < 3) return false;

    int neighbor_count = 0;       // The number of neighbors of vertex v.
    int neighbors[MAX_VERTICES];  // The neighboring vertices of v.

    // Find all undirected neighbors of apex.
    for (int e = 0; e < edge_count; e++) {
      if (edges[e].head_vertex == UNDIRECTED && (edges[e].vertex_set & (1 << apex)) != 0) {
        // The edge contains the given vertex apex. Find the other vertex u.
        int u = __builtin_ctz(static_cast<uint32>(edges[e].vertex_set & ~(1 << apex)));
        neighbors[neighbor_count] = u;
        ++neighbor_count;
      }
    }
    assert(neighbor_count >= 3);

    // Loop through all sets of 3 undirected neighbors of apex, try to find a good base.
    for (uint16 neighbor_mask = 0; neighbor_mask < (1 << neighbor_count); neighbor_mask++) {
      // Ignore unless we have a 3-vertex set.
      if (__builtin_popcount(neighbor_mask) != 3) continue;

      // First transform the neighbor index mask into vertex mask.
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

      // Find all edges in the base.
      int base_edge_indices[3]{0};
      int base_edge_count = 0;
      for (int e = 0; e < edge_count; e++) {
        if (__builtin_popcount(edges[e].vertex_set & vertex_mask) == 2) {
          base_edge_indices[base_edge_count++] = e;
        }
      }
      // Ignore if there are less than 3 edges in the base.
      if (base_edge_count < 3) continue;
      // Check if the base contains a transitive triangle as a subgraph.
      // Recall the subgraph definition: a subgraph can be obtained by
      // (1) deleting vertices
      // (2) deleting edges
      // (3) changing undirected edges to a directed edges.
      //
      // If there is any undirected edge, we have a transitive base.
      if (edges[base_edge_indices[0]].head_vertex == UNDIRECTED ||
          edges[base_edge_indices[1]].head_vertex == UNDIRECTED ||
          edges[base_edge_indices[2]].head_vertex == UNDIRECTED) {
        return true;
      }
      // Now all 3 edges are directed, we have a transitive base as long as it's not a 3-cycle.
      if (edges[base_edge_indices[0]].head_vertex == edges[base_edge_indices[1]].head_vertex ||
          edges[base_edge_indices[1]].head_vertex == edges[base_edge_indices[2]].head_vertex ||
          edges[base_edge_indices[0]].head_vertex == edges[base_edge_indices[2]].head_vertex) {
        return true;
      }
    }
    // All search done, we didn't find a K4D3.
    return false;
  };

  // Check v as apex.
  if (contains_K4D3_apex(v)) return true;
  // Check all vertices that are connected to v by undirected edge.
  for (int e = 0; e < edge_count; e++) {
    if (edges[e].head_vertex == UNDIRECTED && (edges[e].vertex_set & (1 << v)) != 0) {
      // The edge contains the given vertex apex. Find the other vertex u.
      int u = __builtin_ctz(static_cast<uint32>(edges[e].vertex_set & ~(1 << v)));
      if (contains_K4D3_apex(u)) return true;
    }
  }

  return false;
}
