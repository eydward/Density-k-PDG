#include "graph.h"

// Helper function for printing vertex list in an edge.
void print_vertices(std::ostream& os, uint8 vertices) {
  constexpr uint8 MAX_VERTEX_COUNT = 8;
  for (int v = 0; v < MAX_VERTEX_COUNT; v++) {
    if ((vertices & 1) != 0) {
      os << v;
    }
    vertices >>= 1;
  }
}

// Utility function to print an edge array to the given output stream.
// Undirected edge is printed as "013" (for vertex set {0,1,3}),
// and directed edge is printed as "013>1" (for vertex set {0,1,3} and head vertex 1).
void Edge::print_edges(std::ostream& os, uint8 edge_count, const Edge edges[]) {
  os << "{";
  bool is_first = true;
  for (int i = 0; i < edge_count; i++) {
    if (!is_first) {
      os << ", ";
    }
    is_first = false;
    print_vertices(os, edges[i].vertex_set);
    if (edges[i].head_vertex != UNDIRECTED) {
      os << ">" << static_cast<int>(edges[i].head_vertex);
    }
  }
  os << "}\n";
}

// Global to all graph instances: number of vertices in each edge.
int Graph::K = 0;
// Global to all graph instances: total number of vertices in each graph.
int Graph::N = 0;
// Global to all graph instances: number of edges in a complete graph.
int Graph::TOTAL_EDGES = 0;

void Graph::set_global_graph_info(int k, int n) {
  K = k;
  N = n;
  TOTAL_EDGES = compute_binom(n, k);
}
