#pragma once

#include <bits/stdc++.h>

#include "fraction.h"
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

// Represents a k-PDG, with the data structure optimized for computing isomorphisms.
// Template parameters:
//   K = number of vertices in each edge.
//   N = Maximum number of vertices in any graph. Note the code makes assumptions
//       that N<=8 by using 8-bit bitmasks. If N>8 the data type must change.
//   MAX_EDGES = Maximum number of possible edges in a graph.
//               We use static allocation to minimize overhead.
// The n vertices in this graph: 0, 1, ..., n-1.
template <int K, int N, int MAX_EDGES>
struct Graph {
  // The constant n choose k. Used to compute theta.
  static constexpr int BINOM_NK = compute_binom(N, K);

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

  // Number of edges that are undirected.
  uint8 undirected_edge_count;

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
  template <int N1, int MAX_EDGES1>
  void copy_without_init(Graph<K, N1, MAX_EDGES1>& g) const;

  // Returns true if this graph is isomorphic to the other.
  template <int N1, int MAX_EDGES1>
  bool is_isomorphic(const Graph<K, N1, MAX_EDGES1>& other) const;

  // Returns true if the two graphs are identical (exactly same edge sets).
  template <int N1, int MAX_EDGES1>
  bool is_identical(const Graph<K, N1, MAX_EDGES1>& other) const;

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
  void print() const;
  void print_concise() const;
};

// Holds all statistical counters to keep track of number of operations during the search.
struct Counters {
  static uint64 graph_inits;
  static uint64 graph_copies;
  static uint64 graph_canonicalize_ops;
  static uint64 graph_isomorphic_tests;
  // Number of isomorphic tests that have to use vertex permutations.
  static uint64 graph_isomorphic_expensive;
  // Number of isomorphic tests that result in no, but had identical hash
  static uint64 graph_isomorphic_hash_no;
  static uint64 graph_identical_tests;
  static uint64 graph_permute_ops;
  static uint64 graph_permute_canonical_ops;
  static uint64 graph_contains_Tk_tests;
  static std::chrono::time_point<std::chrono::steady_clock> start_time;

  // Start the stopwatch, which will be used by print_counters to calculate elapsed time.
  static void start_stopwatch();
  // Prints the counter values to console.
  static void print_counters();
};
