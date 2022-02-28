#include "graph.h"

// Helper function for debug print().
static void print_vertices(uint8 vertices) {
  for (int v = 0; v < N; v++) {
    if ((vertices & 1) != 0) {
      cout << v;
    }
    vertices >>= 1;
  }
}

bool Graph::edge_allowed(uint8 vertices) const {
  for (int i = 0; i < edge_count; i++) {
    if (vertices == edges[i].vertex_set) return false;
  }
  return true;
}

void Graph::add_edge(uint8 vset, uint8 head) {
  assert(head == UNDIRECTED || ((1 << head) & vset) != 0);
  edges[edge_count++] = Edge(vset, head);
}

void Graph::init() {
  // Sort edges
  sort(edges, edges + edge_count,
       [](const Edge& a, const Edge& b) { return a.vertex_set < b.vertex_set; });

  // Compute signatures of vertices, first pass (degrees, but not hash code).
  // As a side product, also gather the neighbor vertex sets of each vertex.
  uint8 neighbors_undirected[N]{0};
  uint8 neighbors_head[N]{0};  // neighbors_head[i]: head vertex is i.
  uint8 neighbors_tail[N]{0};

  for (int i = 0; i < edge_count; i++) {
    uint8 head = edges[i].head_vertex;
    for (int v = 0; v < N; v++) {
      uint8 mask = 1 << v;
      if ((edges[i].vertex_set & mask) != 0) {
        if (head == UNDIRECTED) {
          vertices[v].degree_undirected++;
          neighbors_undirected[v] |= (edges[i].vertex_set & ~(1 << v));
        } else if (head == v) {
          vertices[v].degree_head++;
          neighbors_head[v] |= (edges[i].vertex_set & ~(1 << v));
        } else {
          vertices[v].degree_tail++;
          neighbors_tail[v] |= (edges[i].vertex_set & ~((1 << v) | (1 << head)));
        }
      }
    }
  }

  // DEBUG print
  for (int v = 0; v < N; v++) {
    cout << "N_undr[" << v << "]: ";
    print_vertices(neighbors_undirected[v]);
    cout << "\nN_head[" << v << "]: ";
    print_vertices(neighbors_head[v]);
    cout << "\nN_tail[" << v << "]: ";
    print_vertices(neighbors_tail[v]);
    cout << "\n";
  }
  // Compute signature of vertices, second pass (neighbor hash).
  // Note we can't update the signatures in the structure during the computation, so use
  // a working copy first.
  uint32 neighbor_hash[N]{0};
  for (int v = 0; v < N; v++) {
  }

  // Finally, compute hash.
  hash = 0;
  // for (const auto& kv : vertices) {
  //  hash ^= kv.first.get_hash() + 0x9E3779B97F4A7C15ull + (hash << 12) + (hash >> 4);
  //}
}

bool Graph::is_isomorphic(const Graph& other) const {
  if (hash != other.hash) return false;
  // TODO
  return true;
}

// Print the graph to the console for debugging purpose.
void Graph::print() const {
  cout << "Graph[" << hex << hash << ", \n";

  bool is_first = true;
  cout << "  undir {";
  for (int i = 0; i < edge_count; i++) {
    if (edges[i].head_vertex == UNDIRECTED) {
      if (!is_first) {
        cout << ", ";
      }
      print_vertices(edges[i].vertex_set);
      is_first = false;
    }
  }

  cout << "}\n  dir   {";
  is_first = true;
  for (int i = 0; i < edge_count; i++) {
    if (edges[i].head_vertex != UNDIRECTED) {
      if (!is_first) {
        cout << ", ";
      }
      print_vertices(edges[i].vertex_set);
      cout << ">" << (int)edges[i].head_vertex;
      is_first = false;
    }
  }
  cout << "},\n";
  for (int v = 0; v < N; v++) {
    cout << "  V[" << v << "]: du=" << (int)vertices[v].degree_undirected
         << ", dh=" << (int)vertices[v].degree_head << ", dt=" << (int)vertices[v].degree_tail
         << ", neighbor=" << vertices[v].neighbor_hash << ", hash=" << hex << vertices[v].get_hash()
         << "\n";
  }
  cout << "]\n";
}