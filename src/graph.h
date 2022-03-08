#pragma once

#include <bits/stdc++.h>

#include "fraction.h"

using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

// Maximum number of vertex allowed in a graph.
constexpr int MAX_VERTICES = 7;
// Maximum number of edges allowed in a graph. Note $35=\binom73=\binom74$.
constexpr int MAX_EDGES = 35;

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
  Edge(uint8 vset, uint8 head) : vertex_set(vset), head_vertex(head) {
#if !NDEBUG
    assert(head_vertex == UNDIRECTED || ((1 << head_vertex) & vertex_set) != 0);
#endif
  }

  // Utility function to print an edge array to the given output stream.
  // Undirected edge is printed as "013" (for vertex set {0,1,3}),
  // and directed edge is printed as "013>1" (for vertex set {0,1,3} and head vertex 1).
  static void print_edges(std::ostream& os, uint8 edge_count, const Edge edges[]);
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

  // Returns a 32-bit code that represents degree information.
  uint32 get_degrees() const {
    return static_cast<uint32>(degree_undirected) << 16 | static_cast<uint32>(degree_head) << 8 |
           static_cast<uint32>(degree_tail);
  }

  // Returns a 32-bit hash code to represent the data.
  uint64 get_hash() const { return *reinterpret_cast<const uint64*>(this); }
};

// Represents a k-PDG, with the data structure optimized for computing isomorphisms.
// The n vertices in this graph: 0, 1, ..., n-1.
struct Graph {
  // Global to all graph instances: number of vertices in each edge.
  static int K;
  // Global to all graph instances: total number of vertices in each graph.
  static int N;
  // Global to all graph instances: number of edges in a complete graph.
  static int TOTAL_EDGES;

  // Set the values of K, N, and TOTAL_EDGES.
  static void set_global_graph_info(int k, int n);

  // The hash code is invariant under isomorphisms.
  uint32 graph_hash;

  // True if the graph is canonicalized (vertex signatures are in decreasing order).
  bool is_canonical;

  // Number of edges in this graph.
  uint8 edge_count;

  // Number of edges that are undirected.
  uint8 undirected_edge_count;

  // The edge set in this graph.
  Edge edges[MAX_EDGES];

  Graph();

  // Resets the current graph to be an empty graph.
  void clear();

  // Returns theta such that (undirected edge density) + theta (directed edge density) = 1.
  // Namely, returns theta = (binom_nk - (undirected edge count)) / (directed edge count).
  Fraction get_theta() const;

  // Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
  // to be added to the graph (this vertex set does not yet exist in the edges).
  bool edge_allowed(uint8 vertices) const;

  // Adds an edge to the graph. It's caller's responsibility to make sure this is allowed.
  // And the input is consistent (head is inside the vertex set).
  void add_edge(Edge edge);

  // Compute the vertex signatures in this graph from the edge set.
  // The result is in the given array.
  void compute_vertex_signature(VertexSignature vs[MAX_VERTICES]) const;
  static void hash_neighbors(uint8 neighbors, const VertexSignature vertices[MAX_VERTICES],
                             uint32& hash_code);

  // Use the given vertex signatures to compute the graph hash and update the graph_hash field.
  void compute_graph_hash(const VertexSignature vs[MAX_VERTICES]);

  // Returns a graph isomorphic to this graph, by applying vertex permutation.
  // The first parameter specifies the permutation. For example p={1,2,0,3} means
  //  0->1, 1->2, 2->0, 3->3.
  // The second parameter is the resulting graph.
  void permute(int p[], Graph& g) const;

  // Similar to permute(), except the current graph must be canonicalized, and the permutation
  // is guaranteed to perserve that.
  void permute_canonical(int p[], Graph& g) const;

  // Canonicalized this graph, so that the vertices are ordered by their signatures.
  // The vertex signatures of the canonicalized graph is returned in the given array.
  void canonicalize(VertexSignature vs[MAX_VERTICES]);

  // Makes a copy of this graph to g.
  void copy(Graph* g) const;

  // Returns true if this graph is isomorphic to the other.
  bool is_isomorphic(const Graph& other) const;

  // Returns true if the two graphs are identical (exactly same edge sets).
  bool is_identical(const Graph& other) const;

  // Returns true if the graph contains the generalized triangle T_k as a subgraph, where
  // v is one of the vertices of the T_k subgraph.
  // T_k is defined as (K+1)-vertex, 3-edge K-graph, with two undirected edges and one directed
  // edge, where all edges share the same set of vertices except for {1,2,3}.
  // For example T_2={12, 13, 23>3}, T_3={124, 134, 234>3}, T_4={1245, 1345, 2345>3}, etc.
  //
  // Note that in k-PDG, subgraph definition is subtle: A is a subgraph of B iff A can be obtained
  // from B, by repeatedly (1) delete a vertex (2) delete an edge (3) forget the direction of
  // an edge.
  //
  // Note: unlike other functions, this function only relies on edges array and therefore
  // can be called without calling init(), which saves time when doing T_k free growing.
  bool contains_Tk(int v) const;

  // Print the graph to the console for debugging purpose.
  void print_concise(std::ostream& os) const;
  void print() const;
};
