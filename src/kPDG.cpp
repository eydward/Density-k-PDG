#include <bits/stdc++.h>
using namespace std;

using uint8 = unsigned __int8;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

// k-PGD, number of vertices in each edge.
constexpr int K = 2;
// Maximum number of vertices in any graph. Note the code makes assumptions
// that N<=8 by using 8-bit bitmasks. If N>8 the data type must change.
constexpr int N = 7;
// Maximum number of possible edges in a graph. Use static allocation to minimize overhead.
constexpr int MAX_EDGES = 36;

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
  uint32 neighbor_hash;     // Combined hash code of the signatures (excluding hashes) of neighbors.

  // Returns a 64-bit hash code to represent the data.
  uint64 get_hash() const { return *reinterpret_cast<const uint64*>(this); }
};

// Represents a k-PDG, with the data structure optimized for computing isomorphisms.
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

  // Returns a hash code of this graph, which is invariant under isomorphisms.
  void init() {
    // Sort edges
    sort(edges, edges + edge_count,
         [](const Edge& a, const Edge& b) { return a.vertex_set < b.vertex_set; });

    // Compute signatures of vertices, first pass (degrees, but not hash code).
    for (int i = 0; i < edge_count; i++) {
      uint8 head = edges[i].head_vertex;
      for (int v = 0; v < N; v++) {
        uint8 mask = 1 << v;
        if ((edges[i].vertex_set & mask) != 0) {
          if (head == UNDIRECTED) {
            vertices[v].degree_undirected++;
          } else if (head == v) {
            vertices[v].degree_head++;
          } else {
            vertices[v].degree_tail++;
          }
        }
      }
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

  // Helper function of print().
  static void print_vertices(uint8 vertices) {
    for (int i = 0; i < N; i++) {
      if ((vertices & 1) != 0) {
        cout << i;
      }
      vertices >>= 1;
    }
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
    for (int i = 0; i < N; i++) {
      cout << "  V[" << i << "]: du=" << (int)vertices[i].degree_undirected
           << ", dh=" << (int)vertices[i].degree_head << ", dt=" << (int)vertices[i].degree_tail
           << ", hash=" << vertices[i].neighbor_hash << "\n";
    }
    cout << "]\n";
  }
};

int main() {
  cout << sizeof(Graph) << "\n";

  Graph g;
  g.add_edge(0b11100, UNDIRECTED);
  g.add_edge(0b1100010, 5);
  g.add_edge(0b1110, 2);
  g.add_edge(0b1011, UNDIRECTED);
  g.init();
  g.print();
  return 0;
}
