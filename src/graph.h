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
  // Combined hash code of the signatures (excluding hashes) of neighbors. Algorithm:
  // Let N_u, N_h, N_t be the neighboring vertex sets that correspond to the 3 degree counts above.
  // Within each set, sort by the signature values (without neighbor_hash value) of the vertices.
  // Then combine the hash with this given order.
  uint32 neighbor_hash;

  uint8 degree_undirected;  // Number of undirected edges through this vertex.
  uint8 degree_head;        // Number of directed edges using this vertex as the head.
  uint8 degree_tail;        // Number of directed edges through this vertex but not as head.
  uint8 reserved;           // Not used, for memory alignment purpose.

  // Returns a 32-bit code that represents degree information.
  uint32 get_degrees() const {
    return static_cast<uint32>(degree_undirected) << 16 | static_cast<uint32>(degree_head) << 8 |
           static_cast<uint32>(degree_tail);
  }

  // Returns a 64-bit hash code to represent the data.
  uint64 get_hash() const { return *reinterpret_cast<const uint64*>(this); }
};

// Combines value into the hash and returns the combined hash.
uint32 hash_combine32(uint32 hash, uint32 value) {
  return hash ^= value + 0x9E3779B9ul + (hash << 6) + (hash >> 2);
}
uint64 hash_combine64(uint64 hash, uint64 value) {
  return hash ^= value + 0x9E3779B97F4A7C15ull + (hash << 12) + (hash >> 4);
}

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

  // True if the graph is initialized (hash, vertices have content).
  bool is_init;
  // True if the graph is canonicalized (vertex signatures are in descreasing order).
  bool is_canonical;

  // Number of vertices that have degrees >0.
  // Only available if is_canonical is true, in which case the rest of the
  // vertices array can be ignored.
  uint8 vertex_count;

  // Number of edges in this graph.
  uint8 edge_count;
  // The edge set in this graph.
  Edge edges[MAX_EDGES];

  // The signatures of each vertex
  VertexSignature vertices[N];

  Graph();

  // Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
  // to be added to the graph (this vertex set does not yet exist in the edges).
  bool edge_allowed(uint8 vertices) const;

  // Add an edge to the graph. It's caller's responsibility to make sure this is allowed.
  // And the input is consistent (head is inside the vertex set).
  void add_edge(uint8 vset, uint8 head);

  // Initializes everything in this graph from the edge set.
  void init();

  void hash_neighbors(uint8 neighbors, uint32& hash);

  // Resets the current graph to no edges.
  void clear();

  // Returns a graph isomorphic to this graph, by applying vertex permutation.
  // The first parameter specifies the permutation. For example p={1,2,0,3} means
  //  0->1, 1->2, 2->0, 3->3.
  // The second parameter is the resulting graph.
  void permute(int p[N], Graph& g) const;

  // Similar to permute(), except the current graph must be canonicalized, and the permutation
  // is guaranteed to perserve that.
  void permute_canonical(int p[N], Graph& g) const;

  // Canonicalized this graph, so that the vertices are ordered by their signatures.
  void canonicalize();

  // Makes a copy of this graph to g.
  void copy(Graph& g) const;

  // Makes a copy of this graph to g, without calling init. The caller can add/remove edges,
  // and must call init() before using g.
  template <int K1, int N1, int MAX_EDGES1>
  void copy_without_init(Graph<K1, N1, MAX_EDGES1>& g) const;

  // Returns true if this graph is isomorphic to the other.
  template <int K1, int N1, int MAX_EDGES1>
  bool is_isomorphic(const Graph<K1, N1, MAX_EDGES1>& other) const;

  // Returns true if the two graphs are identical (exactly same edge sets).
  template <int K1, int N1, int MAX_EDGES1>
  bool is_identical(const Graph<K1, N1, MAX_EDGES1>& other) const;

  // Print the graph to the console for debugging purpose.
  void print() const;
  void print_concise() const;
};

#include "graph.hpp"