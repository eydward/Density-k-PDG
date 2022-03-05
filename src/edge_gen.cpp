#include "edge_gen.h"

// Initializes the generator for the given new vertex count.
// k = number of vertices in each edge.
void EdgeGenerator::initialize(int k, int vertex_count) {
  n = vertex_count;
  current_mask = 0;
  edge_count = 0;
  edge_candidates.clear();
  for (uint8 mask = 0; mask < (1 << (n - 1)); mask++) {
    if (__builtin_popcount(mask) == k - 1) {
      edge_candidates.push_back(mask | (1 << (n - 1)));
    }
  }
  assert(edge_candidates.size() == compute_binom(n - 1, k - 1));
}

void EdgeGenerator::reset_enumeration() {
  current_mask = 0;
  edge_count = 0;
}

bool EdgeGenerator::next() {
  if (edge_count > 0) {
    // If we already have a set of edges, and are in the middle of trying all the
    // combinations of directions in that set of edges, we'll just continue to do so here.
    for (int i = 0; i < edge_count; i++) {
      // Find the next head vertex to try, for this edge.
      do {
        ++edges[i].head_vertex;
      } while (edges[i].head_vertex < n &&
               ((1 << edges[i].head_vertex) & edges[i].vertex_set) == 0);

      // If we've found a head vertex to try, just return. Otherwise, set the head to
      // UNDIRECTED and let the loop continue to the next edge.
      if (edges[i].head_vertex < n) {
        return true;
      } else {
        edges[i].head_vertex = UNDIRECTED;
      }
    }
  }

  // If it reaches here, either we just started the enumeration process, or we are done
  // with the previous set of edges. So generate another set of edges to try.
  ++current_mask;
  if (current_mask == (1 << edge_candidates.size())) {
    // We've exhausted all edge set combinations, return false to stop the process.
    return false;
  }
  // Create the edge set using the binary mask.
  edge_count = 0;
  for (int i = 0; i < edge_candidates.size(); i++) {
    if ((current_mask & (1 << i)) != 0) {
      edges[edge_count].vertex_set = edge_candidates[i];
      edges[edge_count].head_vertex = UNDIRECTED;
      ++edge_count;
    }
  }
  return true;
}