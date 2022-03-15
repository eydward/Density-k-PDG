#include "graph.h"

#include "counters.h"
#include "permutator.h"

// Combines value into the hash and returns the combined hash.
uint32 hash_combine32(uint32 hash_code, uint32 value) {
  return hash_code ^= value + 0x9E3779B9ul + (hash_code << 6) + (hash_code >> 2);
}
uint64 hash_combine64(uint64 hash, uint64 value) {
  return hash ^= value + 0x9E3779B97F4A7C15ull + (hash << 12) + (hash >> 4);
}

// Helper function for printing vertex list in an edge.
void print_vertices(std::ostream& os, uint8 vertices) {
  constexpr uint8 MAX_VERTEX_COUNT = 8;
  for (int v = 0; v < MAX_VERTEX_COUNT; v++) {
    if ((vertices & 1) != 0) {
      os << v;
    }
    vertices >>= 1;
  }
}

// Utility function to print an edge array to the given output stream.
// Undirected edge is printed as "013" (for vertex set {0,1,3}),
// and directed edge is printed as "013>1" (for vertex set {0,1,3} and head vertex 1).
void Edge::print_edges(std::ostream& os, uint8 edge_count, const Edge edges[]) {
  os << "{";
  bool is_first = true;
  for (int i = 0; i < edge_count; i++) {
    if (!is_first) {
      os << ", ";
    }
    is_first = false;
    print_vertices(os, edges[i].vertex_set);
    if (edges[i].head_vertex != UNDIRECTED) {
      os << ">" << static_cast<int>(edges[i].head_vertex);
    }
  }
  os << "}\n";
}

// Utility function to print an array of VertexSignatures to the given output stream,
// for debugging purpose.
void VertexSignature::print_vertices(std::ostream& os,
                                     const VertexSignature vertices[MAX_VERTICES]) {
  os << "  {";
  bool is_first = true;
  for (int v = 0; v < MAX_VERTICES; v++) {
    if (vertices[v].deg.get_degrees() != 0) {
      if (!is_first) {
        os << ", ";
      }
      is_first = false;
      os << "V[" << v << "]=(" << static_cast<int>(vertices[v].deg.degree_undirected) << ", "
         << static_cast<int>(vertices[v].deg.degree_head) << ", "
         << static_cast<int>(vertices[v].deg.degree_tail) << ", " << vertices[v].neighbor_hash
         << ")";
    }
  }
  os << "}\n";
}

// Utility function to print an degree info array to the output stream, for debugging purpose.
void DegreeInfo::print_degree_info(std::ostream& os, const DegreeInfo degs[MAX_VERTICES], int m) {
  os << "  {";
  for (int i = 0; i < m; i++) {
    if (i > 0) {
      os << ", ";
    }
    os << "d[" << i << "]=(" << static_cast<int>(degs[i].degree_undirected) << ", "
       << static_cast<int>(degs[i].degree_head) << ", " << static_cast<int>(degs[i].degree_tail)
       << ")";
  }
  os << "}\n";
}

// Global to all graph instances: number of vertices in each edge.
int Graph::K = 0;
// Global to all graph instances: total number of vertices in each graph.
int Graph::N = 0;
// Global to all graph instances: number of edges in a complete graph.
int Graph::TOTAL_EDGES = 0;
// Global to all graph instances: pre-computed the vertex masks, used in
// various computations including compute_codegree_signature().
VertexMask Graph::VERTEX_MASKS[MAX_VERTICES]{0};

void Graph::set_global_graph_info(int k, int n) {
  K = k;
  N = n;
  TOTAL_EDGES = compute_binom(n, k);
  for (int m = 1; m <= k; m++) {
    VertexMask& mask = VERTEX_MASKS[m];
    mask.mask_count = 0;
    for (uint8 bits = 0; bits < (1 << n); bits++) {
      if (__builtin_popcount(bits) == m) {
        mask.masks[mask.mask_count++] = bits;
      }
    }
  }
}

Graph::Graph() : graph_hash(0), is_canonical(false), edge_count(0), undirected_edge_count(0) {}

// Resets the current graph to be an empty graph.
void Graph::clear() {
  is_canonical = false;
  graph_hash = edge_count = undirected_edge_count = 0;
}

// Returns theta such that (undirected edge density) + theta (directed edge density) = 1.
// Namely, returns theta = (binom_nk - (undirected edge count)) / (directed edge count).
Fraction Graph::get_theta() const {
  uint8 directed = edge_count - undirected_edge_count;
  if (directed > 0) {
    return Fraction(TOTAL_EDGES - undirected_edge_count, directed);
  } else {
    return Fraction(1E8, 1);
  }
}

// Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
// to be added to the graph (this vertex set does not yet exist in the edges).
bool Graph::edge_allowed(uint8 vertices) const {
  for (int i = 0; i < edge_count; i++) {
    if (vertices == edges[i].vertex_set) return false;
  }
  return true;
}

// Add an edge to the graph. It's caller's responsibility to make sure this is allowed.
// And the input is consistent (head is inside the vertex set).
void Graph::add_edge(Edge edge) {
#if !NDEBUG
  assert(edge_allowed(edge.vertex_set));
  assert(__builtin_popcount(edge.vertex_set) == K);
#endif
  edges[edge_count++] = edge;
  if (edge.head_vertex == UNDIRECTED) {
    ++undirected_edge_count;
  }
}

void Graph::generate_graph_hash(GraphInvariants& gi, int codeg_vertices) {
  compute_vertex_signature(gi.vertices);
  uint32 hash = compute_graph_hash(gi.vertices);
  uint32 codegree_hash = compute_codegree_signature(gi.codegrees, codeg_vertices);
  if (codegree_hash != 0) {
    hash = hash_combine32(hash, codegree_hash);
  }
  graph_hash = hash;
}

// Initializes everything in this graph from the edge set.
void Graph::compute_vertex_signature(VertexSignature vs[MAX_VERTICES]) const {
  Counters::increment_compute_vertex_signatures();

  static_assert(sizeof(VertexSignature) == 8);
  for (int v = 0; v < MAX_VERTICES; v++) {
    *reinterpret_cast<uint64*>(&vs[v]) = 0;
  }

  // Compute signatures of vertices, first pass (degrees, but not hash code).
  // As a side product, also gather the neighbor vertex sets of each vertex.
  uint8 neighbors_undirected[MAX_VERTICES]{0};
  uint8 neighbors_head[MAX_VERTICES]{0};  // neighbors_head[i]: head vertex is i.
  uint8 neighbors_tail[MAX_VERTICES]{0};

  for (int i = 0; i < edge_count; i++) {
    uint8 head = edges[i].head_vertex;
    for (int v = 0; v < N; v++) {
      uint8 mask = 1 << v;
      if ((edges[i].vertex_set & mask) != 0) {
        if (head == UNDIRECTED) {
          vs[v].deg.degree_undirected++;
          neighbors_undirected[v] |= (edges[i].vertex_set & ~(1 << v));
        } else if (head == v) {
          vs[v].deg.degree_head++;
          neighbors_head[v] |= (edges[i].vertex_set & ~(1 << v));
        } else {
          vs[v].deg.degree_tail++;
          neighbors_tail[v] |= (edges[i].vertex_set & ~(1 << v));
        }
      }
    }
  }

  // Compute signature of vertices, second pass (neighbor hash).
  // Note we can't update the signatures in the structure during the computation, so use
  // a working copy first.
  for (int v = 0; v < N; v++) {
    int neighbor_count = 0;
    hash_neighbors(neighbors_undirected[v], vs, vs[v].neighbor_hash);
    hash_neighbors(neighbors_head[v], vs, vs[v].neighbor_hash);
    hash_neighbors(neighbors_tail[v], vs, vs[v].neighbor_hash);
  }
}

uint32 Graph::compute_codegree_signature(DegreeInfo degs[MAX_EDGES], int codeg_vertices) const {
  if (codeg_vertices < 2 || codeg_vertices > K) return 0;
  const VertexMask& mask = VERTEX_MASKS[codeg_vertices];
  for (int m = 0; m < mask.mask_count; m++) {
    degs[m] = {0};
  }

  for (int i = 0; i < edge_count; i++) {
    uint8 head = 1 << edges[i].head_vertex;
    for (int m = 0; m < mask.mask_count; m++) {
      uint8 bits = mask.masks[m];
      if ((edges[i].vertex_set & bits) == bits) {
        if (head == UNDIRECTED) {
          degs[m].degree_undirected++;
        } else if ((head & bits) != 0) {
          degs[m].degree_head++;
        } else {
          degs[m].degree_tail++;
        }
      }
    }
  }

  std::sort(degs, degs + mask.mask_count, [](const DegreeInfo& a, const DegreeInfo& b) {
    return a.get_degrees() < b.get_degrees();
  });
  uint32 hash = 0;
  for (int m = 0; m < mask.mask_count; m++) {
    hash = hash_combine32(hash, degs[m].get_degrees());
  }
  return hash;
}

void Graph::hash_neighbors(uint8 neighbors, const VertexSignature vs[MAX_VERTICES],
                           uint32& hash_code) {
  // The working buffer to compute hash in deterministic order.
  uint32 signatures[MAX_VERTICES];
  if (neighbors == 0) {
    hash_code = hash_combine32(hash_code, 0x12345678);
  } else {
    int neighbor_count = 0;
    for (int i = 0; neighbors != 0; i++) {
      if ((neighbors & 0x1) != 0) {
        signatures[neighbor_count++] = vs[i].deg.get_degrees();
      }
      neighbors >>= 1;
    }

    // Sort to make hash combination process invariant to isomorphisms.
    if (neighbor_count > 1) {
      std::sort(signatures, signatures + neighbor_count);
    }
    for (int i = 0; i < neighbor_count; i++) {
      hash_code = hash_combine32(hash_code, signatures[i]);
    }
  }
}

// Use the given vertex signatures to compute the graph hash and update the graph_hash field.
uint32 Graph::compute_graph_hash(const VertexSignature vs[MAX_VERTICES]) const {
  uint64 hash = 0;
  uint64 signatures[MAX_VERTICES];
  for (int v = 0; v < N; v++) {
    signatures[v] = vs[v].get_hash();
  }
  // Note we sort by descreasing order. Then hash everything in.
  std::sort(signatures, signatures + N, std::greater<uint64>());
  for (int v = 0; v < N; v++) {
    hash = hash_combine64(hash, signatures[v]);
  }
  return (hash >> 32) ^ hash;
}

// Returns a graph isomorphic to this graph, by applying vertex permutation.
// The first parameter specifies the permutation. For example p={1,2,0,3} means
//  0->1, 1->2, 2->0, 3->3.
// The second parameter is the resulting graph.
void Graph::permute(int p[], Graph& g, int codeg_vertices) const {
  Counters::increment_graph_permute_ops();

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
  g.undirected_edge_count = undirected_edge_count;
  g.finalize_edges();
  GraphInvariants gi;
  g.generate_graph_hash(gi, codeg_vertices);
}

void Graph::permute_edges(int p[], Graph& g) const {
  g.edge_count = edge_count;
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
}
void Graph::permute_canonical(int p[], Graph& g) const {
  Counters::increment_graph_permute_canonical_ops();

  assert(is_canonical);
  permute_edges(p, g);
  g.finalize_edges();

  g.graph_hash = graph_hash;
  g.is_canonical = is_canonical;
  g.undirected_edge_count = undirected_edge_count;
}

// Returns the canonicalized graph in g, where the vertices are ordered by their signatures.
void Graph::canonicalize(GraphInvariants& gi, int codeg_vertices) {
  Counters::increment_graph_canonicalize_ops();

  // Compute the signatures before canonicalization.
  generate_graph_hash(gi, codeg_vertices);

  // First get sorted vertex indices by the vertex signatures.
  // Note we sort by descreasing order, to push vertices to lower indices.
  int s[MAX_VERTICES];
  for (int v = 0; v < N; v++) s[v] = v;
  const VertexSignature* vs = gi.vertices;
  std::sort(s, s + N, [vs](int a, int b) { return vs[a].get_hash() > vs[b].get_hash(); });
  // Now compute the inverse, which gives the permutation used to canonicalize.
  int p[MAX_VERTICES];
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

  finalize_edges();
  is_canonical = true;
}

// Call either this function, or canonicalize(), after all edges are added. This allows
// isomorphism checks to be performed. The operation in this function is included in
// canonicalize() so there is no need to call this function if canonicalize() is used.
void Graph::finalize_edges() {
  // Sort edges
  std::sort(edges, edges + edge_count,
            [](const Edge& a, const Edge& b) { return a.vertex_set < b.vertex_set; });
}

// Makes a copy of this graph to g.
void Graph::copy(Graph* g) const {
  Counters::increment_graph_copies();

  g->graph_hash = graph_hash;
  g->is_canonical = is_canonical;
  g->edge_count = edge_count;
  g->undirected_edge_count = undirected_edge_count;
  for (int i = 0; i < edge_count; i++) {
    g->edges[i] = edges[i];
  }
}

// Returns true if this graph is isomorphic to the other.
bool Graph::is_isomorphic(const Graph& other) const {
  Counters::increment_graph_isomorphic_tests();
  assert(is_canonical);
  assert(other.is_canonical);

  if (edge_count != other.edge_count || undirected_edge_count != other.undirected_edge_count ||
      graph_hash != other.graph_hash) {
    return false;
  }

  // Opportunistic check, just in case the two graphs are identical without doing any permutation.
  if (is_identical(other)) {
    Counters::increment_graph_isomorphic_true();
    return true;
  }
  Counters::increment_graph_isomorphic_expensive();

  // Now all the easy checks are done, have to permute and check.
  // The set of vertices with the same signature can permute among themselves.
  // If the two graphs are identical, then we've found isomorphism. Otherwise after
  // all permutations are tried, we declare the two graphs as non-isomorphic.
  VertexSignature vs[MAX_VERTICES];
  compute_vertex_signature(vs);

  std::vector<std::pair<int, int>> perm_sets;
  for (int v = 0; v < N - 1 && vs[v].deg.get_degrees() > 0; v++) {
    if (vs[v + 1].get_hash() == vs[v].get_hash()) {
      int t = v;
      while (t < N && vs[t].get_hash() == vs[v].get_hash()) {
        t++;
      }
      perm_sets.push_back(std::make_pair(v, t));
      v = t;
    }
  }
  if (perm_sets.size() > 0) {
    Permutator perm(move(perm_sets));
    Graph h;
    while (perm.next()) {
      permute_canonical(perm.p, h);
      if (h.is_identical(other)) {
        // if (codeg_hash_this != codeg_hash_other) {
        //   std::cout << "Is-isomorphic but different codeg hash: " << codeg_hash_this << ", "
        //             << codeg_hash_other << "\n";
        //   pa->print();
        //   DegreeInfo::print_degree_info(std::cout, degs_this,
        //                                 codegree_masks[codeg_vcount].mask_count);
        //   pb->print();
        //   DegreeInfo::print_degree_info(std::cout, degs_other,
        //                                 codegree_masks[codeg_vcount].mask_count);
        // }
        // assert(codeg_hash_this == codeg_hash_other);
        Counters::increment_graph_isomorphic_true();
        return true;
      }
    }
  }

  Counters::increment_graph_isomorphic_hash_no();

#if false
  std::cout << "Non-isomorphic with match hashes:\n";
  pa->print();
  pb->print();
#endif
  return false;
}

bool Graph::is_isomorphic_slow(const Graph& other) const {
  // Opportunistic check, just in case the two graphs are identical without doing any permutation.
  if (is_identical(other)) {
    return true;
  }
  // Bruteforce all permutations of the vertices, and check whether the permuted graph
  // is identical to `other`.
  int perm[MAX_VERTICES];
  for (int v = 0; v < N; v++) {
    perm[v] = v;
  }
  Graph copy;
  while (std::next_permutation(perm, perm + N)) {
    permute_edges(perm, copy);
    copy.finalize_edges();
    if (copy.is_identical(other)) {
      return true;
    }
  }
  // std::cout << "Non-isomorphic slow " << N << ":\n";
  // print();
  // other.print();
  return false;
}

// Returns true if the two graphs are identical (exactly same edge sets).
bool Graph::is_identical(const Graph& other) const {
  Counters::increment_graph_identical_tests();

  if (edge_count != other.edge_count) return false;
  for (int i = 0; i < edge_count; i++) {
    if (edges[i].vertex_set != other.edges[i].vertex_set ||
        edges[i].head_vertex != other.edges[i].head_vertex)
      return false;
  }
  return true;
}

// Returns true if the graph contains the generalized triangle T_k as a subgraph, where
// v is one of the vertices of the T_k subgraph.
// T_k is defined as (K+1)-vertex, 3-edge K-graph, with two undirected edges and one directed
// edge, where all edges share the same set of vertices except for {1,2,3}.
// For example T_2={12, 13, 23>3}, T_3={124, 134, 234>3}, T_4={1245, 1345, 2345>3}, etc.
//
// Note that in k-PDG, subgraph definition is subtle: A is a subgraph of B iff A can be obtained
// from B, by repeatedly (1) delete a vertex (2) delete an edge (3) forget the direction of
// an edge.
bool Graph::contains_Tk(int v) const {
  uint64 counter = Counters::increment_graph_contains_Tk_tests();

  // There are two possibilities that $v \in T_k \subseteq H$.
  // (1) v is in the "triangle with stem cut off". Namely:
  //     exists vertices $x,y$, and vertex set $S$ with size $K-2$, $S$ disjoint
  //     from $v,x,y$, such that there are 3 edges: $S\cup vx, S\cup vy, S\cup xy$,
  //     and at least one of the 3 edges is directed, with the head being one of $v,x,y$.
  // (2) v is in the "common stem". Namely:
  //     exists vertices $x,y,z$, and vertex set $S$ with size $K-3$ and $x\in S$, $S$ disjoint
  //     from $x,y,z$, such that there are 3 edges: $S\cup xy, S\cup yz, S\cup zx$,
  //     and at least one of the 3 edges is directed, with the head being one of $x,y,z$.

  // Check for the two possibilities. For the most part, the checking logic is same.
  // Example to understand the code below:
  //                      possibility (1)         possibility (2)
  //                      S = {3,4,5,6}           S = {3,4,5,6}
  //                      K=6, v=2, x=1, y=0.     K=6, v=4, x=2, y=1, z=0.
  //   e_i  = 01111110    $S\cup vx$              $S\cup xy$
  //   e_j  = 01111101    $S\cup vy$              $S\cup yz$
  //   m    = 00000011
  //   mask = 01111111
  //   e_k  = 01111011    $S\cup xy$              $S\cup zx$
  //   stem = 01111000
  //   xyz  = 00000111
  for (int i = 0; i < edge_count - 1; i++) {
    uint8 e_i = edges[i].vertex_set;
    if ((e_i & (1 << v)) == 0) continue;

    for (int j = i + 1; j < edge_count; j++) {
      uint8 e_j = edges[j].vertex_set;
      if ((e_j & (1 << v)) == 0) continue;

      uint8 m = e_i ^ e_j;
      if (__builtin_popcount(m) == 2) {
        uint8 mask = m | edges[i].vertex_set;
        for (int k = 0; k < edge_count; k++) {
          if (k == i || k == j) continue;

          uint8 e_k = edges[k].vertex_set;
          if (__builtin_popcount(mask ^ e_k) == 1) {
            uint8 stem = m ^ e_k;
            uint8 xyz = (e_i | e_j | e_k) & ~stem;
            if ((edges[i].head_vertex != UNDIRECTED && (xyz & (1 << edges[i].head_vertex)) != 0) ||
                (edges[j].head_vertex != UNDIRECTED && (xyz & (1 << edges[j].head_vertex)) != 0) ||
                (edges[k].head_vertex != UNDIRECTED && (xyz & (1 << edges[k].head_vertex)) != 0)) {
              // if (counter % 97 == 0) {
              //   std::cout << "contains_Tk for v=" << v << " : true B :(i,j,k)=" << i << j << k
              //             << "\n  ";
              //   print_concise(std::cout);
              // }
              return true;
            }
          }
        }
      }
    }
  }
  // if (counter % 97 == 0) {
  //   std::cout << "contains_Tk for v=" << v << " : false :\n  ";
  //   print_concise(std::cout);
  // }
  return false;
}

void Graph::print_concise(std::ostream& os) const { Edge::print_edges(os, edge_count, edges); }

// Print the graph to the console for debugging purpose.
void Graph::print() const {
  std::cout << "Graph ~ " << graph_hash << ", canonical=" << is_canonical
            << ", eg_cnt=" << (int)edge_count << ", undir_eg_cnt=" << (int)undirected_edge_count
            << ", \n  ";
  print_concise(std::cout);
  VertexSignature vs[MAX_VERTICES];
  compute_vertex_signature(vs);
  VertexSignature::print_vertices(std::cout, vs);
}
