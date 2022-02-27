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
// Special value to indicate an edge is undirected.
constexpr uint8 UNDIRECTED = 0xFF;

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

  // The map from signatures to the set of vertices in a bitmask.
  map<VertexSignature, uint8> vertices;

  // The edge set in this graph.
  // The key is a bitmask of all vertices in the edge. Example 00001011 means vertices {0,1,3}.
  // The value is the id of the head vertex if the edge is directed,
  // or UNDIRECTED if the edge is undirected.
  map<uint8, uint8> edges;

  // Returns a hash code of this graph, which is invariant under isomorphisms.
  void init() {
    // Compute signatures of vertices.

    // Finally, compute hash.
    hash = 0;
    for (const auto& kv : vertices) {
      hash ^= kv.first.get_hash() + 0x9E3779B97F4A7C15ull + (hash << 12) + (hash >> 4);
    }
  }

  // Returns true if this graph is isomorphic to the other.
  bool is_isomorphic(const Graph& other) const {
    if (hash != other.hash) return false;
    // TODO
    return true;
  }
};

int main() {
  cout << sizeof(VertexSignature) << sizeof(size_t);
  return 0;
}
