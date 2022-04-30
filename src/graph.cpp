#include "graph.h"

#include "counters.h"
#include "permutator.h"

// Combines value into the hash and returns the combined hash.
uint64 hash_combine64(uint64 hash, uint64 value) {
  return hash ^= value + 0x9E3779B97F4A7C15ull + (hash << 12) + (hash >> 4);
}

// Returns the text representation of the vertex id (one of "0123456789ab")
char vertex_id_to_char(int vertex_id) {
  assert(0 <= vertex_id && vertex_id < Graph::N);
  if (vertex_id == 10) {
    return 'a';
  } else if (vertex_id == 11) {
    return 'b';
  } else {
    return '0' + vertex_id;
  }
}

// Helper function for printing vertex list in an edge. `vertices` is a bitmask.
void print_vertices(std::ostream& os, uint16 vertices) {
  for (int v = 0; v < MAX_VERTICES; v++) {
    if ((vertices & 1) != 0) {
      os << vertex_id_to_char(v);
    }
    vertices >>= 1;
  }
}

Edge::Edge() : vertex_set(0), head_vertex(0) {}
Edge::Edge(uint16 vset, uint8 head) : vertex_set(vset), head_vertex(head) {
  assert((vset & 0xF000) == 0);
  assert(head_vertex == UNDIRECTED || ((static_cast<uint16>(1) << head_vertex) & vertex_set) != 0);
}

// Utility function to print an edge array to the given output stream.
// Undirected edge is printed as "013" (for vertex set {0,1,3}),
// and directed edge is printed as "013>1" (for vertex set {0,1,3} and head vertex 1).
void Edge::print_edges(std::ostream& os, uint8 edge_count, const Edge edges[], bool aligned) {
  os << "{";
  bool is_first = true;
  for (uint8 i = 0; i < edge_count; i++) {
    if (!is_first) {
      os << ", ";
    }
    is_first = false;
    print_vertices(os, edges[i].vertex_set);
    if (edges[i].head_vertex != UNDIRECTED) {
      os << ">" << vertex_id_to_char(edges[i].head_vertex);
    } else if (aligned) {
      os << "  ";
    }
  }
  os << "}\n";
}

// Reset all data fields to 0, except setting the vertex_id using the given vid value.
void VertexSignature::reset(int vid) {
  degree_undirected = degree_head = degree_tail = 0;
  vertex_id = vid;
}

// Returns the degree info, which encodes the values of all 3 degree fields,
// but does not contain the vertex id.
uint32 VertexSignature::get_degrees() const {
  return static_cast<uint32>(degree_undirected) << 16 | (static_cast<uint32>(degree_head) << 8) |
         (static_cast<uint32>(degree_tail));
}

// Utility function to print an array of VertexSignatures to the given output stream,
// for debugging purpose.
void VertexSignature::print_vertices(std::ostream& os,
                                     const VertexSignature vertices[MAX_VERTICES]) {
  os << "  {";
  bool is_first = true;
  for (int v = 0; v < MAX_VERTICES; v++) {
    if (vertices[v].get_degrees() != 0) {
      if (!is_first) {
        os << ", ";
      }
      is_first = false;
      os << "V[" << v << "]=(" << static_cast<int>(vertices[v].vertex_id) << ", "
         << static_cast<int>(vertices[v].degree_undirected) << ", "
         << static_cast<int>(vertices[v].degree_head) << ", "
         << static_cast<int>(vertices[v].degree_tail) << ", " << std::hex
         << vertices[v].get_degrees() << ", " << std::dec << ")";
    }
  }
  os << "}\n";
}

// Global to all graph instances: number of vertices in each edge.
int Graph::K = 0;
// Global to all graph instances: total number of vertices in each graph.
int Graph::N = 0;
// Global to all graph instances: number of edges in a complete graph.
int Graph::TOTAL_EDGES = 0;
// Global to all graph instances: pre-computed the vertex masks, used in various computations.
VertexMask Graph::VERTEX_MASKS[MAX_VERTICES + 1]{0};

void Graph::set_global_graph_info(int k, int n) {
  K = k;
  N = n;
  TOTAL_EDGES = compute_binom(n, k);
  for (int m = 1; m <= k; m++) {
    VertexMask& mask = VERTEX_MASKS[m];
    mask.mask_count = 0;
    for (uint16 bits = 0; bits < (1 << n); bits++) {
      if (__builtin_popcount(bits) == m) {
        mask.masks[mask.mask_count++] = bits;
      }
    }
    assert(mask.mask_count == compute_binom(n, m));
  }
}

Graph::Graph() : graph_hash(0), is_canonical(false), edge_count(0), undirected_edge_count(0) {}

// Returns theta such that (undirected edge density) + theta (directed edge density) = 1.
// Namely, returns theta = (binom_nk - (undirected edge count)) / (directed edge count).
Fraction Graph::get_theta() const {
  uint8 directed = edge_count - undirected_edge_count;
  if (directed > 0) {
    return Fraction(TOTAL_EDGES - undirected_edge_count, directed);
  } else {
    return Fraction::infinity();
  }
}

// Returns the hash of this graph.
uint32 Graph::get_graph_hash() const {
  assert(is_canonical);
  return graph_hash;
}

// Returns true if the edge specified by the bitmask of the vertices in the edge is allowed
// to be added to the graph (this vertex set does not yet exist in the edges).
bool Graph::edge_allowed(uint16 vertices) const {
  for (uint8 i = 0; i < edge_count; i++) {
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

// Initializes everything in this graph from the edge set.
void Graph::compute_vertex_signature() {
  Counters::increment_compute_vertex_signatures();

  static_assert(sizeof(VertexSignature) == 4);
  for (int v = 0; v < MAX_VERTICES; v++) {
    vertices[v].reset(v);
  }
  // Compute signatures of vertices, loop through the edges to calculate vertice degree info.
  for (int i = 0; i < edge_count; i++) {
    uint8 head = edges[i].head_vertex;
    for (int v = 0; v < N; v++) {
      uint16 mask = static_cast<uint16>(1) << v;
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
}

// Returns a graph isomorphic to this graph, by applying vertex permutation.
// The first parameter specifies the permutation. For example p={1,2,0,3} means
//  0->1, 1->2, 2->0, 3->3.
// The second parameter is the resulting graph.
void Graph::permute_for_testing(int p[], Graph& g) const {
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
  // Copy the vertices
  for (int v = 0; v < N; v++) {
    g.vertices[v] = vertices[v];
  }
  g.edge_count = edge_count;
  g.undirected_edge_count = undirected_edge_count;
  g.finalize_edges();
  g.is_canonical = is_canonical;
  g.graph_hash = graph_hash;
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

// Performs a permutation of the vertices according to the given p array on this graph.
// The first parameter specifies the permutation. For example p={1,2,0,3} means
//  0->1, 1->2, 2->0, 3->3.
// The second parameter is the resulting graph.
// This graph must be canonicalized, and the permutation is guaranteed to perserve that.
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
void Graph::canonicalize() {
  Counters::increment_graph_canonicalize_ops();

  // Compute the signatures before canonicalization.
  compute_vertex_signature();
  // First get sorted vertex indices by the vertex signatures.
  // Note we sort by descreasing order, to push vertices to lower indices.
  std::sort(vertices, vertices + N, [this](const VertexSignature& a, const VertexSignature& b) {
    return a.get_degrees() > b.get_degrees();
  });

  // Now compute the inverse, which gives the permutation used to canonicalize.
  int p[MAX_VERTICES];
  for (int v = 0; v < N; v++) {
    p[vertices[v].vertex_id] = v;
  }

  uint64 hash = 0;
  for (int v = 0; v < N; v++) {
    hash = hash_combine64(hash, vertices[v].get_degrees());
  }
  graph_hash = (hash >> 32) ^ hash;

  for (int i = 0; i < edge_count; i++) {
    uint16 vset = edges[i].vertex_set;
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

// Copy the edge info of this graph to g. It does not copy vertex signatures and graph hash.
void Graph::copy_edges(Graph& g) const {
  Counters::increment_graph_copies();

  g.graph_hash = 0;
  g.is_canonical = false;
  g.edge_count = edge_count;
  g.undirected_edge_count = undirected_edge_count;
  for (int i = 0; i < edge_count; i++) {
    g.edges[i] = edges[i];
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

  std::vector<std::pair<int, int>> perm_sets;
  for (int v = 0; v < N - 1 && vertices[v].get_degrees() > 0; v++) {
    if (vertices[v + 1].get_degrees() == vertices[v].get_degrees()) {
      int t = v;
      while (t < N && vertices[t].get_degrees() == vertices[v].get_degrees()) {
        t++;
      }
      perm_sets.push_back(std::make_pair(v, t));
      v = t - 1;
    }
  }

  if (perm_sets.size() > 0) {
    Permutator perm(move(perm_sets));
    Graph h;
    while (perm.next()) {
      permute_canonical(perm.p, h);
      if (h.is_identical(other)) {
        Counters::increment_graph_isomorphic_true();
        return true;
      }
    }
  }

  Counters::increment_graph_isomorphic_hash_no();
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
  Counters::increment_graph_contains_Tk_tests();

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
    uint16 e_i = edges[i].vertex_set;
    if ((e_i & (1 << v)) == 0) continue;

    for (int j = i + 1; j < edge_count; j++) {
      uint16 e_j = edges[j].vertex_set;
      if ((e_j & (1 << v)) == 0) continue;

      uint16 m = e_i ^ e_j;
      if (__builtin_popcount(m) == 2) {
        uint16 mask = m | edges[i].vertex_set;
        for (int k = 0; k < edge_count; k++) {
          if (k == i || k == j) continue;

          uint16 e_k = edges[k].vertex_set;
          if (__builtin_popcount(mask ^ e_k) == 1) {
            uint16 stem = m ^ e_k;
            uint16 xyz = (e_i | e_j | e_k) & ~stem;
            if ((edges[i].head_vertex != UNDIRECTED && (xyz & (1 << edges[i].head_vertex)) != 0) ||
                (edges[j].head_vertex != UNDIRECTED && (xyz & (1 << edges[j].head_vertex)) != 0) ||
                (edges[k].head_vertex != UNDIRECTED && (xyz & (1 << edges[k].head_vertex)) != 0)) {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

// Used to establish a deterministic order when growing the search tree.
// Since this is called infrequently, its speed is not important. We want deterministic behavior
// and an intuitive ordering for human inspection of the detailed log.
bool Graph::operator<(const Graph& other) const {
  if (edge_count < other.edge_count) return true;
  if (edge_count > other.edge_count) return false;
  for (uint8 i = 0; i < edge_count; i++) {
    if (edges[i].vertex_set < other.edges[i].vertex_set) return true;
    if (edges[i].vertex_set > other.edges[i].vertex_set) return false;
    int head_this =
        edges[i].head_vertex == UNDIRECTED ? -1 : static_cast<int>(edges[i].head_vertex);
    int head_other = other.edges[i].head_vertex == UNDIRECTED
                         ? -1
                         : static_cast<int>(other.edges[i].head_vertex);
    if (head_this < head_other) return true;
    if (head_this > head_other) return false;
  }
  return false;
}

void Graph::print_concise(std::ostream& os, bool aligned) const {
  Edge::print_edges(os, edge_count, edges, aligned);
}

// Print the graph to the console for debugging purpose.
void Graph::print() const {
  std::cout << "Graph ~ " << graph_hash << ", canonical=" << is_canonical
            << ", eg_cnt=" << (int)edge_count << ", undir_eg_cnt=" << (int)undirected_edge_count
            << ", \n  ";
  print_concise(std::cout, true);
  VertexSignature::print_vertices(std::cout, vertices);
}

std::string Graph::serialize_edges() const {
  std::stringstream oss;
  print_concise(oss, false);
  std::string text = oss.str();
  return text.substr(0, text.length() - 1);  // Removes the tailing line return.
}

// Returns the vertex id if the given char is a valid representation of a vertext
// (one of "0123456789ab", and less than Graph::N). Otherwise returns -1.
int parse_vertex_char(char c) {
  int value = -1;
  if ('0' <= c && c <= '9') {
    value = c - '0';
  } else if (c == 'a') {
    value = 10;
  } else if (c == 'b') {
    value = 11;
  }
  if (value >= Graph::N) {
    value = -1;
  }
  return value;
}

bool Graph::parse_edges(const std::string& edge_representation, Graph& result) {
  result.is_canonical = false;
  result.edge_count = result.undirected_edge_count = 0;

  if (edge_representation.length() < 2) return false;
  if (edge_representation[0] != '{') return false;
  if (edge_representation[edge_representation.length() - 1] != '}') return false;

  std::string s = edge_representation.substr(1, edge_representation.length() - 2);
  size_t prev_pos = 0;
  while (s.length() != 0) {
    size_t pos = s.find(',', prev_pos);
    std::string e = pos == s.npos ? s.substr(prev_pos) : s.substr(prev_pos, pos - prev_pos);
    uint16 vertex_set = 0;
    uint8 head = UNDIRECTED;
    for (size_t i = 0; i < e.length(); i++) {
      char c = e[i];
      if (c == ' ') continue;
      int vertex_id = parse_vertex_char(c);
      if (vertex_id >= 0) {
        vertex_set |= (1 << vertex_id);
      }
      if (c == '>') {
        if (i != e.length() - 2) return false;
        vertex_id = parse_vertex_char(e[i + 1]);
        if (vertex_id < 0) return false;
        head = vertex_id;
        if ((vertex_set & (1 << head)) == 0) return false;
      }
    }
    if (__builtin_popcount(vertex_set) != Graph::K) return false;
    result.add_edge(Edge(vertex_set, head));
    if (pos == s.npos) break;
    prev_pos = pos + 1;
  }
  return true;
}