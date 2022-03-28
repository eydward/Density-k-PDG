#pragma once

#include <bits/stdc++.h>

#include "fraction.h"

using int8 = int8_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// Maximum number of vertex allowed in a graph.
constexpr int MAX_VERTICES = 8;
// Maximum number of edges allowed in a graph. Note $56=\binom83$ which allows
// all (K,N) combinations with N<=8, except for K=4,N=8, which is not computationally feasible
// in our current approach anyway.
constexpr int MAX_EDGES = 56;

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

  // Utility function to print an edge array to the given output stream, for debugging purpose.
  // Undirected edge is printed as "013" (for vertex set {0,1,3}),
  // and directed edge is printed as "013>1" (for vertex set {0,1,3} and head vertex 1).
  // If aligned==true, pad the undirected edges, so the print is easier to read.
  static void print_edges(std::ostream& os, uint8 edge_count, const Edge edges[], bool aligned);
};
static_assert(sizeof(Edge) == 2);

// Represent the characteristics of a vertex.
// Both get_degrees() and get_hash() are invariant under graph isomorphisms.
struct VertexSignature {
  // Number of directed edges through the given vertex set, with the head not in the set.
  uint8 degree_tail;
  // Number of directed edges through the given vertex set, with the head in the given vertex head.
  uint8 degree_head;
  // Number of undirected edges through the give vertex set.
  uint8 degree_undirected;
  // The vertex id. This is not used in get_hash() in order to maintain invariant property.
  uint8 vertex_id;

  // Reset all data fields to 0, except setting the vertex_id using the given vid value.
  void reset(int vid) {
    degree_undirected = degree_head = degree_tail = 0;
    vertex_id = vid;
  }

  // Utility function to print an degree info array to the output stream, for debugging purpose.
  uint32 get_degrees() const {
    return static_cast<uint32>(degree_undirected) << 16 | (static_cast<uint32>(degree_head) << 8) |
           (static_cast<uint32>(degree_tail));
  }

  // Utility function to print an array of VertexSignatures to the given output stream,
  // for debugging purpose.
  static void print_vertices(std::ostream& os, const VertexSignature vertices[MAX_VERTICES]);
};
static_assert(sizeof(VertexSignature) == 4);

// Represents the bitmasks of vertices, used to in various computations such as codegree info.
// Each VertexMask struct instance holds all valid vertex bitmasks for a given k value.
struct VertexMask {
  // Number of valid masks in the next array.
  uint8 mask_count;
  // Each element in this array has exactly k bits that are 1s. The position of the 1-bits
  // indicate which vertex should be used in the computations.
  uint8 masks[MAX_EDGES];
};

// Represents a k-PDG, with the data structure optimized for computing isomorphisms.
// The n vertices in this graph: 0, 1, ..., n-1.
struct Graph {
 public:
  // Global to all graph instances: number of vertices in each edge.
  static int K;
  // Global to all graph instances: total number of vertices in each graph.
  static int N;
  // Global to all graph instances: number of edges in a complete graph.
  static int TOTAL_EDGES;
  // Global to all graph instances: pre-computed the vertex masks, used in
  // various computations including compute_codegree_signature().
  static VertexMask VERTEX_MASKS[MAX_VERTICES + 1];

  // Set the values of K, N, and TOTAL_EDGES.
  static void set_global_graph_info(int k, int n);

  // Parses the edge representation into a Graph object. Returns true if successful.
  // Used for testing purpose.
  static bool parse_edges(const std::string& edge_representation, Graph& result);

 private:
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

  // Information of the vertices
  VertexSignature vertices[MAX_VERTICES];

 public:
  Graph();

  // Returns theta such that (undirected edge density) + theta (directed edge density) = 1.
  // Namely, returns theta = (binom_nk - (undirected edge count)) / (directed edge count).
  Fraction get_theta() const;

  // Returns the hash of this graph.
  uint32 get_graph_hash() const {
    assert(is_canonical);
    return graph_hash;
  }
  uint8 get_edge_count() const { return edge_count; }
  uint8 get_undirected_edge_count() const { return undirected_edge_count; }
  uint8 get_directed_edge_count() const { return edge_count - undirected_edge_count; }

  // Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
  // to be added to the graph (this vertex set does not yet exist in the edges).
  bool edge_allowed(uint8 vertices) const;

  // Adds an edge to the graph. It's caller's responsibility to make sure this is allowed.
  // And the input is consistent (head is inside the vertex set).
  void add_edge(Edge edge);

  // Performs a permutation of the vertices according to the given p array on this graph.
  // The first parameter specifies the permutation. For example p={1,2,0,3} means
  //  0->1, 1->2, 2->0, 3->3.
  // The second parameter is the resulting graph.
  // This graph must be canonicalized, and the permutation is guaranteed to perserve that.
  void permute_canonical(int p[], Graph& g) const;

  // Canonicalizes this graph, so that the vertices are ordered by their signatures.
  // This function also computes the graph_hash field.
  void canonicalize();

  // Copy the edge info of this graph to g. It does not copy vertex signatures and graph hash.
  void copy_edges(Graph& g) const;

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

  // Return true if the graphs contains a complete graph with 4 vertices with at least
  // one directed edge. Only works for 2-PDG. The given vertex v must be in the complete graph.
  //
  // The name "K4" here refers to complete graph. Unfortunately we also use K to refer
  // to the number of vertices in each edge in this code, don't be confused.
  bool contains_K4(int v) const;

  // Print the graph to the output stream for debugging purpose.
  // If aligned==true, pad the undirected edges, so the print is easier to read.
  void print_concise(std::ostream& os, bool aligned) const;
  // Print the graph to the console for debugging purpose.
  void print() const;
  // Returns the edge representation of this graph, for testing purpose.
  std::string serialize_edges() const;

  // Used to establish a deterministic order when growing the search tree.
  bool operator<(const Graph& other) const;

 private:
  // Call either this function, or canonicalize(), after all edges are added. This allows
  // isomorphism checks to be performed. The operation in this function is included in
  // canonicalize() so there is no need to call this function if canonicalize() is used.
  void finalize_edges();

  // Computes the vertex signatures in this graph from the edge set.
  // The result is in the given array.
  void compute_vertex_signature();

  // Perform a permutation of the vertices of this graph according to the p array, put in `g`.
  // Only set the data in the edges array in `g` without touching other fields.
  void permute_edges(int p[], Graph& g) const;

  // Returns a graph isomorphic to this graph, by applying vertex permutation.
  // The first parameter specifies the permutation. For example p={1,2,0,3} means
  //  0->1, 1->2, 2->0, 3->3.
  // The second parameter is the resulting graph.
  // This is only used in unit tests to verify the correctness of implementation of other functions.
  void permute_for_testing(int p[], Graph& g) const;

  // Do not use any optimizations or shortcuts, just mechanically check whether the two
  // graphs are isomorphic by bruteforce permuting the vertices. This is not used in the
  // actual run, but is used in self-test and verifying the correctness of the optimized
  // algorithm.
  bool is_isomorphic_slow(const Graph& other) const;

  // Friend declarations that allows unit testing of some private implementations.
#define FRIEND_TEST(test_case_name, test_name) friend class test_case_name##_##test_name##_Test
  FRIEND_TEST(GraphTest, PermuteIsomorphic);
  FRIEND_TEST(GraphTest, PermuteCanonical);
  FRIEND_TEST(GraphTest, Canonicalize);
  FRIEND_TEST(GraphTest, Canonicalize2);
  FRIEND_TEST(GraphTest, Canonicalize3);
  FRIEND_TEST(GraphTest, ContainsT3);
  FRIEND_TEST(GraphTest, Copy);
  FRIEND_TEST(GraphTest, NotContainsT3);
  FRIEND_TEST(GraphTest, IsomorphicSlow);
  FRIEND_TEST(GraphTest, Isomorphic_B);
  FRIEND_TEST(GraphTest, Isomorphic_C);
  FRIEND_TEST(GraphTest, GraphDataStructure);
  FRIEND_TEST(GraphTest, T3);
  FRIEND_TEST(EdgeGeneratorTest, Generate22);
  FRIEND_TEST(EdgeGeneratorTest, Generate23);
  FRIEND_TEST(EdgeGeneratorTest, Generate23WithSkip);
  FRIEND_TEST(EdgeGeneratorTest, Generate33);
  FRIEND_TEST(EdgeGeneratorTest, Generate45);
  friend class IsomorphismStressTest;
};
static_assert(sizeof(Graph) == 152);
