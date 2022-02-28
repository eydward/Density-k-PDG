#pragma once

#include <bits/stdc++.h>
using namespace std;

using uint8 = unsigned __int8;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

// Special value to indicate an edge is undirected.
constexpr uint8 UNDIRECTED = 0xFF;

// Specifies one edge in the graph. The vertex_set is a bitmasks of all vertices in the edge.
// Example 00001011 means vertices {0,1,3}.
// The head_vertex is the id of the head vertex if the edge is directed,
// or UNDIRECTED if the edge is undirected.
struct Edge {
  uint8 vertex_set;
  uint8 head_vertex;

  Edge() : vertex_set(0), head_vertex(0) {}
  Edge(uint8 vset, uint8 head) : vertex_set(vset), head_vertex(head) {}
};

// Represent the characteristics of a vertex, that is invariant under graph isomorphisms.
struct VertexSignature {
  uint8 degree_undirected;  // Number of undirected edges through this vertex.
  uint8 degree_head;        // Number of directed edges using this vertex as the head.
  uint8 degree_tail;        // Number of directed edges through this vertex but not as head.
  uint8 reserved;           // Not used, for memory alignment purpose.

  // Combined hash code of the signatures (excluding hashes) of neighbors. Algorithm:
  // Let N_u, N_h, N_t be the neighboring vertex sets that correspond to the 3 degree counts above.
  // Within each set, sort by the signature values (without neighbor_hash value) of the vertices.
  // Then combine the hash with this given order.
  uint32 neighbor_hash;

  // Returns a 64-bit hash code to represent the data.
  uint64 get_hash() const { return *reinterpret_cast<const uint64*>(this); }
};

// Represents a k-PDG, with the data structure optimized for computing isomorphisms.
// Template parameters:
//   K = number of vertices in each edge.
//   N = Maximum number of vertices in any graph. Note the code makes assumptions
//       that N<=8 by using 8-bit bitmasks. If N>8 the data type must change.
//   MAX_EDGES = Maximum number of possible edges in a graph.
//               We use static allocation to minimize overhead.

template <int K, int N, int MAX_EDGES>
struct Graph {
  // n vertices in this graph: 0, 1, ..., n-1.

  // The hash code is invariant under isomorphisms.
  uint64 hash;

  // The signatures of each vertex
  VertexSignature vertices[N];

  // Number of edges in this graph.
  uint8 edge_count;

  // The edge set in this graph.
  Edge edges[MAX_EDGES];

  Graph() : hash(0), vertices{}, edge_count(0), edges{} {}

  // Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
  // to be added to the graph (this vertex set does not yet exist in the edges).
  bool edge_allowed(uint8 vertices) const {
    for (int i = 0; i < edge_count; i++) {
      if (vertices == edges[i].vertex_set) return false;
    }
    return true;
  }

  // Add an edge to the graph. It's caller's responsibility to make sure this is allowed.
  // And the input is consistent (head is inside the vertex set).
  void add_edge(uint8 vset, uint8 head) {
    assert(head == UNDIRECTED || ((1 << head) & vset) != 0);
    edges[edge_count++] = Edge(vset, head);
  }

  // Initializes everything in this graph from the edge set.
  void init() {
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

  // Returns true if this graph is isomorphic to the other.
  bool is_isomorphic(const Graph& other) const {
    if (hash != other.hash) return false;
    // TODO
    return true;
  }

  // Print the graph to the console for debugging purpose.
  void print() const {
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
           << ", neighbor=" << vertices[v].neighbor_hash << ", hash=" << hex
           << vertices[v].get_hash() << "\n";
    }
    cout << "]\n";
  }

  // Helper function for debug print().
  static void print_vertices(uint8 vertices) {
    for (int v = 0; v < N; v++) {
      if ((vertices & 1) != 0) {
        cout << v;
      }
      vertices >>= 1;
    }
  }
};
