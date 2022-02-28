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

  // The signatures of each vertex
  VertexSignature vertices[N];

  // Number of edges in this graph.
  uint8 edge_count;

  // True if the graph is canonicalized (vertex signatures are in descreasing order).
  bool is_canonical;

  // Number of vertices that have degrees >0.
  // Only available if is_canonical is true, in which case the rest of the
  // vertices array can be ignored.
  uint8 vertex_count;

  // The edge set in this graph.
  Edge edges[MAX_EDGES];

  Graph() : hash(0), vertices{}, edge_count(0), is_canonical(false), vertex_count(0) {}

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
#if !NDEBUG
    assert(head == UNDIRECTED || ((1 << head) & vset) != 0);
    assert(edge_allowed(vset));
    assert(__builtin_popcount(vset) == K);
#endif
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

#if false
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
#endif

    // Compute signature of vertices, second pass (neighbor hash).
    // Note we can't update the signatures in the structure during the computation, so use
    // a working copy first.
    uint32 neighbor_hash[N]{0};
    for (int v = 0; v < N; v++) {
      int neighbor_count = 0;
      hash_neighbors(neighbors_undirected[v], neighbor_hash[v]);
      hash_neighbors(neighbors_head[v], neighbor_hash[v]);
      hash_neighbors(neighbors_tail[v], neighbor_hash[v]);
    }
    // Now copy the working values back into the vertex signatures
    for (int v = 0; v < N; v++) {
      vertices[v].neighbor_hash = neighbor_hash[v];
    }

    // Finally, compute hash for the entire graph.
    hash = 0;
    uint64 signatures[N];
    for (int v = 0; v < N; v++) {
      signatures[v] = vertices[v].get_hash();
    }
    // Note we sort by descreasing order, to put heavily used vertices at the beginning.
    sort(signatures, signatures + N, greater<uint64>());
    // Stop the hash combination once we reach vertices with 0 degrees.
    for (int v = 0; v < N && (signatures[v] >> 32 != 0); v++) {
      hash = hash_combine64(hash, signatures[v]);
    }
  }

  void hash_neighbors(uint8 neighbors, uint32& hash) {
    // The working buffer to compute hash in deterministic order.
    uint32 signatures[N];
    if (neighbors == 0) {
      hash = hash_combine32(hash, 0x12345678);
    } else {
      int neighbor_count = 0;
      for (int i = 0; neighbors != 0; i++) {
        if ((neighbors & 0x1) != 0) {
          signatures[neighbor_count++] = vertices[i].get_degrees();
        }
        neighbors >>= 1;
      }

      // Sort to make hash combination process invariant to isomorphisms.
      if (neighbor_count > 1) {
        sort(signatures, signatures + neighbor_count);
      }
      for (int i = 0; i < neighbor_count; i++) {
        hash = hash_combine32(hash, signatures[i]);
      }
    }
  }

  // Resets the current graph to no edges.
  void clear() {
    hash = 0;
    edge_count = 0;
    is_canonical = false;
    vertex_count = 0;
    for (int v = 0; v < N; v++) {
      *reinterpret_cast<uint64*>(&vertices[v]) = 0;
    }
  }

  // Returns a graph isomorphic to this graph, by applying vertex permutation.
  // The first parameter specifies the permutation. For example p={1,2,0,3} means
  //  0->1, 1->2, 2->0, 3->3.
  // The second parameter is the resulting graph.
  void permute(int p[N], Graph& g) const {
    g.clear();
    // Copy the edges with permutation.
    for (int i = 0; i < edge_count; i++) {
      if (edges[i].head_vertex == UNDIRECTED) {
        g.edges[i].head_vertex = UNDIRECTED;
      } else {
        g.edges[i].head_vertex = p[edges[i].head_vertex];
      }
      g.edges[i].vertex_set = 0;
      for (int v = 0; v < N; v++) {
        if ((edges[i].vertex_set & (1 << v)) != 0) {
          g.edges[i].vertex_set |= (1 << p[v]);
        }
      }
    }
    g.edge_count = edge_count;
    g.init();
  }

  // Returns the canonicalized graph in g, where the vertices are ordered by their signatures.
  void canonicalize(Graph& g) const {
    // First get sorted vertex indices by the vertex signatures.
    // Note we sort by descreasing order, to push vertices to lower indices.
    int s[N];
    for (int v = 0; v < N; v++) s[v] = v;
    sort(s, s + N,
         [this](int a, int b) { return vertices[a].get_hash() > vertices[b].get_hash(); });
    // Now compute the inverse, which gives the permutation used to canonicalize.
    int p[N];
    for (int v = 0; v < N; v++) {
      p[s[v]] = v;
    }
    permute(p, g);
    g.is_canonical = true;
    for (int v = 0; v < N; v++) {
      if (g.vertices[v].get_degrees() != 0) {
        g.vertex_count++;
      } else {
        break;
      }
    }
  }

  // Makes a copy of this graph to g, without calling init. The caller can add/remove edges,
  // and must call init() before using g.
  template <int K1, int N1, int MAX_EDGES1>
  void copy_without_init(Graph<K1, N1, MAX_EDGES1>& g) const {
    static_assert(K <= K1 && N <= N1 && MAX_EDGES <= MAX_EDGES1);
    for (int i = 0; i < edge_count; i++) {
      g.edges[i] = edges[i];
    }
    g.edge_count = edge_count;
  }

  // Returns true if this graph is isomorphic to the other.
  template <int K1, int N1, int MAX_EDGES1>
  bool is_isomorphic(const Graph<K1, N1, MAX_EDGES1>& other) const {
    if (edge_count != other.edge_count || hash != other.hash) return false;

    // Canonicalize if necessary.
    const Graph *pa, *pb;
    if (is_canonical) {
      pa = this;
    } else {
      Graph<K, N, MAX_EDGES> c;
      canonicalize(c);
      pa = &c;
    }
    if (other.is_canonical) {
      pb = &other;
    } else {
      Graph<K1, N1, MAX_EDGES1> other_c;
      canonicalize(other_c);
      pb = &other_c;
    }

    // Verify vertex signatures are same.
    if (pa->vertex_count != pb->vertex_count) return false;
    for (int v = 0; v < pa->vertex_count; v++) {
      if (pa->vertices[v].get_hash() != pb->vertices[v].get_hash()) return false;
    }

    // Opportunistic check: if after canonicalization, the two graphs are identical,
    // then they are isomorphic.
    if (pa->is_identical(*pb)) return true;

    // Now all the easy checks are done, have to permute and check.
    // The set of vertices with the same signature can permute among themselves.
    // If the two graphs are identical, then we've found isomorphism. Otherwise after
    // all permutations are tried, we declare the two graphs as non-isomorphic.
    // TODO
    cout << "**** WARNING: UNIMPLEMENTED final isomorphism check:\n";
    pa->print();
    pb->print();
    return false;
  }

  // Returns true if the two graphs are identical (exactly same edge sets).
  template <int K1, int N1, int MAX_EDGES1>
  bool is_identical(const Graph<K1, N1, MAX_EDGES1>& other) const {
    if (edge_count != other.edge_count) return false;
    for (int i = 0; i < edge_count; i++) {
      if (edges[i].vertex_set != other.edges[i].vertex_set ||
          edges[i].head_vertex != other.edges[i].head_vertex)
        return false;
    }
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
           << vertices[v].get_hash() << ", canonical=" << is_canonical
           << ", vcnt=" << (int)vertex_count << "\n";
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
