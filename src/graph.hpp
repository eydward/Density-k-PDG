#pragma once
#include "permutator.h"

// Helper function for debug print().
void print_vertices(uint8 vertices, int N) {
  for (int v = 0; v < N; v++) {
    if ((vertices & 1) != 0) {
      cout << v;
    }
    vertices >>= 1;
  }
}

template <int K, int N, int MAX_EDGES>
Graph<K, N, MAX_EDGES>::Graph()
    : hash(0), is_init(false), is_canonical(false), vertex_count(0), edge_count(0), vertices{} {}

// Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
// to be added to the graph (this vertex set does not yet exist in the edges).
template <int K, int N, int MAX_EDGES>
bool Graph<K, N, MAX_EDGES>::edge_allowed(uint8 vertices) const {
  for (int i = 0; i < edge_count; i++) {
    if (vertices == edges[i].vertex_set) return false;
  }
  return true;
}

// Add an edge to the graph. It's caller's responsibility to make sure this is allowed.
// And the input is consistent (head is inside the vertex set).
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::add_edge(uint8 vset, uint8 head) {
#if !NDEBUG
  assert(head == UNDIRECTED || ((1 << head) & vset) != 0);
  assert(edge_allowed(vset));
  assert(__builtin_popcount(vset) == K);
#endif
  edges[edge_count++] = Edge(vset, head);
}

// Initializes everything in this graph from the edge set.
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::init() {
  ++Counters::graph_inits;

  if (is_init) {
    hash = 0;
    is_canonical = false;
    vertex_count = 0;
    for (int v = 0; v < N; v++) {
      *reinterpret_cast<uint64*>(&vertices[v]) = 0;
    }
  } else {
    is_init = true;
  }

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
      print_vertices(neighbors_undirected[v], N);
      cout << "\nN_head[" << v << "]: ";
      print_vertices(neighbors_head[v], N);
      cout << "\nN_tail[" << v << "]: ";
      print_vertices(neighbors_tail[v], N);
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

template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::hash_neighbors(uint8 neighbors, uint32& hash) {
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
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::clear() {
  hash = 0;
  is_init = is_canonical = false;
  edge_count = vertex_count = 0;
  for (int v = 0; v < N; v++) {
    *reinterpret_cast<uint64*>(&vertices[v]) = 0;
  }
}

// Returns a graph isomorphic to this graph, by applying vertex permutation.
// The first parameter specifies the permutation. For example p={1,2,0,3} means
//  0->1, 1->2, 2->0, 3->3.
// The second parameter is the resulting graph.
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::permute(int p[N], Graph& g) const {
  ++Counters::graph_permute_ops;

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

template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::permute_canonical(int p[N], Graph& g) const {
  ++Counters::graph_permute_canonical_ops;

  assert(is_canonical);
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
  sort(g.edges, g.edges + edge_count,
       [](const Edge& a, const Edge& b) { return a.vertex_set < b.vertex_set; });

  g.hash = hash;
  g.is_init = is_init;
  g.is_canonical = is_canonical;
  g.vertex_count = vertex_count;
  g.edge_count = edge_count;
  for (int v = 0; v < N; v++) {
    g.vertices[v] = vertices[v];
  }
}

// Returns the canonicalized graph in g, where the vertices are ordered by their signatures.
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::canonicalize() {
  ++Counters::graph_canonicalize_ops;

  // First get sorted vertex indices by the vertex signatures.
  // Note we sort by descreasing order, to push vertices to lower indices.
  int s[N];
  for (int v = 0; v < N; v++) s[v] = v;
  sort(s, s + N, [this](int a, int b) { return vertices[a].get_hash() > vertices[b].get_hash(); });
  // Now compute the inverse, which gives the permutation used to canonicalize.
  int p[N];
  for (int v = 0; v < N; v++) {
    p[s[v]] = v;
  }

  for (int i = 0; i < edge_count; i++) {
    uint8 vset = edges[i].vertex_set;
    if (edges[i].head_vertex != UNDIRECTED) {
      edges[i].head_vertex = p[edges[i].head_vertex];
    }
    edges[i].vertex_set = 0;
    for (int v = 0; v < N; v++) {
      if ((vset & (1 << v)) != 0) {
        edges[i].vertex_set |= (1 << p[v]);
      }
    }
  }

  init();
  is_canonical = true;
  for (int v = 0; v < N; v++) {
    if (vertices[v].get_degrees() != 0) {
      vertex_count++;
    } else {
      break;
    }
  }
}

// Makes a copy of this graph to g.
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::copy(Graph& g) const {
  ++Counters::graph_copies;

  g.hash = hash;
  for (int v = 0; v < N; v++) {
    *reinterpret_cast<uint64*>(&g.vertices[v]) = *reinterpret_cast<const uint64*>(&vertices[v]);
  }
  g.is_init = is_init;
  g.is_canonical = is_canonical;
  g.vertex_count = vertex_count;
  g.edge_count = edge_count;
  for (int i = 0; i < edge_count; i++) {
    g.edges[i] = edges[i];
  }
}

// Makes a copy of this graph to g, without calling init. The caller can add/remove edges,
// and must call init() before using g.
template <int K, int N, int MAX_EDGES>
template <int K1, int N1, int MAX_EDGES1>
void Graph<K, N, MAX_EDGES>::copy_without_init(Graph<K1, N1, MAX_EDGES1>& g) const {
  ++Counters::graph_copies;

  static_assert(K <= K1 && N <= N1 && MAX_EDGES <= MAX_EDGES1);
  for (int i = 0; i < edge_count; i++) {
    g.edges[i] = edges[i];
  }
  g.edge_count = edge_count;
}

// Returns true if this graph is isomorphic to the other.
template <int K, int N, int MAX_EDGES>
template <int K1, int N1, int MAX_EDGES1>
bool Graph<K, N, MAX_EDGES>::is_isomorphic(const Graph<K1, N1, MAX_EDGES1>& other) const {
  ++Counters::graph_isomorphic_tests;

  if (edge_count != other.edge_count || hash != other.hash) return false;

  // Canonicalize if necessary.
  Graph this_copy, other_copy;
  const Graph *pa, *pb;
  if (is_canonical) {
    pa = this;
  } else {
    copy(this_copy);
    this_copy.canonicalize();
    pa = &this_copy;
  }
  if (other.is_canonical) {
    pb = &other;
  } else {
    other.copy(other_copy);
    other_copy.canonicalize();
    pb = &other_copy;
  }

  // Verify vertex signatures are same.
  if (pa->vertex_count != pb->vertex_count) return false;
  for (int v = 0; v < pa->vertex_count; v++) {
    if (pa->vertices[v].get_hash() != pb->vertices[v].get_hash()) return false;
  }

  // Opportunistic check: if after canonicalization, the two graphs are identical,
  // then they are isomorphic.
  if (pa->is_identical(*pb)) return true;

  ++Counters::graph_isomorphic_expensive;

  // Now all the easy checks are done, have to permute and check.
  // The set of vertices with the same signature can permute among themselves.
  // If the two graphs are identical, then we've found isomorphism. Otherwise after
  // all permutations are tried, we declare the two graphs as non-isomorphic.

  vector<pair<int, int>> perm_sets;
  for (int v = 0; v < N - 1 && vertices[v].get_degrees() > 0; v++) {
    if (vertices[v + 1].get_hash() == vertices[v].get_hash()) {
      int t = v;
      while (t < N && vertices[t].get_hash() == vertices[v].get_hash()) {
        t++;
      }
      perm_sets.push_back(make_pair(v, t));
      v = t;
    }
  }
  if (perm_sets.size() > 0) {
    Permutator<N> perm(move(perm_sets));
    Graph h;
    while (perm.next()) {
      pa->permute_canonical(perm.p, h);
      if (h.is_identical(*pb)) return true;
    }
  }

  ++Counters::graph_isomorphic_hash_no;

#if !NDEBUG
  cout << "**** WARNING: final isomorphism check:\n";
  pa->print();
  pb->print();
#endif
  return false;
}

// Returns true if the two graphs are identical (exactly same edge sets).
template <int K, int N, int MAX_EDGES>
template <int K1, int N1, int MAX_EDGES1>
bool Graph<K, N, MAX_EDGES>::is_identical(const Graph<K1, N1, MAX_EDGES1>& other) const {
  ++Counters::graph_identical_tests;

  if (edge_count != other.edge_count) return false;
  for (int i = 0; i < edge_count; i++) {
    if (edges[i].vertex_set != other.edges[i].vertex_set ||
        edges[i].head_vertex != other.edges[i].head_vertex)
      return false;
  }
  return true;
}

// Print the graph to the console for debugging purpose.
template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::print() const {
  cout << "Graph[" << hex << hash << ", init=" << is_init << ", canonical=" << is_canonical
       << ", vcnt=" << (int)vertex_count << ", \n";

  bool is_first = true;
  cout << "  undir {";
  for (int i = 0; i < edge_count; i++) {
    if (edges[i].head_vertex == UNDIRECTED) {
      if (!is_first) {
        cout << ", ";
      }
      print_vertices(edges[i].vertex_set, N);
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
      print_vertices(edges[i].vertex_set, N);
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

template <int K, int N, int MAX_EDGES>
void Graph<K, N, MAX_EDGES>::print_concise() const {
  cout << "  {";
  bool is_first = true;
  for (int i = 0; i < edge_count; i++) {
    if (!is_first) {
      cout << ", ";
    }
    is_first = false;
    print_vertices(edges[i].vertex_set, N);
    if (edges[i].head_vertex != UNDIRECTED) {
      cout << ">" << (int)edges[i].head_vertex;
    }
  }
  cout << "}\n";
}
