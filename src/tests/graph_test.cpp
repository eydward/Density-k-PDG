#include "../graph.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

// Helper used in parse_edges().
#define throw_assert(c) \
  if (!(c)) throw std::invalid_argument(#c);

// Returns the text representation of the edges in the graph.
std::string serialize_edges(const Graph& g) {
  std::stringstream oss;
  g.print_concise(oss);
  std::string text = oss.str();
  throw_assert(text[text.length() - 1] == '\n');
  return text.substr(0, text.length() - 1);
}

// Returns a graph, constructed from the text representation of the edges. For example from input
//    "{123>2, 013}"
// A 3-graph with 2 edges will be constructed, the first edge is directed with 2 as the head,
// the second edge is undirected.
//
// This is used to facilitate unit tests.
Graph parse_edges(const std::string& text) {
  throw_assert(text[0] == '{');
  throw_assert(text[text.length() - 1] == '}');
  std::string s = text.substr(1, text.length() - 2);
  Graph g;
  int prev_pos = 0;
  while (s.length() != 0) {
    int pos = s.find(',', prev_pos);
    std::string e = pos == s.npos ? s.substr(prev_pos) : s.substr(prev_pos, pos - prev_pos);
    uint8 vertex_set = 0;
    uint8 head = UNDIRECTED;
    for (int i = 0; i < e.length(); i++) {
      char c = e[i];
      if (c == ' ') continue;
      if ('0' <= c && c <= '6') {
        vertex_set |= (1 << (c - '0'));
      }
      if (c == '>') {
        throw_assert(i == e.length() - 2);
        c = e[i + 1];
        throw_assert('0' <= c && c <= '6');
        head = c - '0';
        throw_assert((vertex_set & (1 << head)) != 0);
      }
    }
    throw_assert(__builtin_popcount(vertex_set) == Graph::K);
    g.add_edge(Edge(vertex_set, head));
    if (pos == s.npos) break;
    prev_pos = pos + 1;
  }
  // Verify the parse produced the same graph as the input.
  throw_assert(serialize_edges(g) == text);
  return g;
}

TEST(GraphTest, Init) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{234, 156>5, 123>2, 013}");

  EXPECT_FALSE(g.is_canonical);

  EXPECT_EQ(4, g.edge_count);
  EXPECT_EQ(2, g.undirected_edge_count);
  EXPECT_EQ(g.edges[0].vertex_set, 0b11100);
  EXPECT_EQ(g.edges[0].head_vertex, UNDIRECTED);
  EXPECT_EQ(g.edges[1].vertex_set, 0b1100010);
  EXPECT_EQ(g.edges[1].head_vertex, 5);
  EXPECT_EQ(g.edges[2].vertex_set, 0b1110);
  EXPECT_EQ(g.edges[2].head_vertex, 2);
  EXPECT_EQ(g.edges[3].vertex_set, 0b1011);
  EXPECT_EQ(g.edges[3].head_vertex, UNDIRECTED);
  EXPECT_EQ(serialize_edges(g), "{234, 156>5, 123>2, 013}");

  VertexSignature vertices[MAX_VERTICES];
  g.compute_vertex_signature(vertices);

  EXPECT_EQ(vertices[0].degree_undirected, 1);
  EXPECT_EQ(vertices[0].degree_head, 0);
  EXPECT_EQ(vertices[0].degree_tail, 0);

  EXPECT_EQ(vertices[1].degree_undirected, 1);
  EXPECT_EQ(vertices[1].degree_head, 0);
  EXPECT_EQ(vertices[1].degree_tail, 2);

  EXPECT_EQ(vertices[2].degree_undirected, 1);
  EXPECT_EQ(vertices[2].degree_head, 1);
  EXPECT_EQ(vertices[2].degree_tail, 0);

  EXPECT_EQ(vertices[3].degree_undirected, 2);
  EXPECT_EQ(vertices[3].degree_head, 0);
  EXPECT_EQ(vertices[3].degree_tail, 1);
}

// Utility function to create and initialize T_3.
Graph get_T3() {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{013, 123>2, 023, 234>2}");
  VertexSignature vertices[MAX_VERTICES];
  g.canonicalize(vertices);
  return g;
}

TEST(GraphTest, T3) {
  Graph g = get_T3();

  EXPECT_EQ(4, g.edge_count);
  // Canonicalization: 1->1, 2->3, 3->0, 4->2, 0->4.
  EXPECT_EQ(g.edges[0].vertex_set, 0b1011);  // 013>3
  EXPECT_EQ(g.edges[0].head_vertex, 3);
  EXPECT_EQ(g.edges[1].vertex_set, 0b1101);  // 023>3
  EXPECT_EQ(g.edges[1].head_vertex, 3);
  EXPECT_EQ(g.edges[2].vertex_set, 0b10011);  // 014
  EXPECT_EQ(g.edges[2].head_vertex, UNDIRECTED);
  EXPECT_EQ(g.edges[3].vertex_set, 0b11001);  // 034
  EXPECT_EQ(g.edges[3].head_vertex, UNDIRECTED);
  EXPECT_EQ(serialize_edges(g), "{013>3, 023>3, 014, 034}");

  VertexSignature vertices[MAX_VERTICES];
  g.compute_vertex_signature(vertices);

  EXPECT_EQ(vertices[0].degree_undirected, 2);
  EXPECT_EQ(vertices[0].degree_head, 0);
  EXPECT_EQ(vertices[0].degree_tail, 2);

  EXPECT_EQ(vertices[1].degree_undirected, 1);
  EXPECT_EQ(vertices[1].degree_head, 0);
  EXPECT_EQ(vertices[1].degree_tail, 1);

  EXPECT_EQ(vertices[2].degree_undirected, 0);
  EXPECT_EQ(vertices[2].degree_head, 0);
  EXPECT_EQ(vertices[2].degree_tail, 1);

  EXPECT_EQ(vertices[3].degree_undirected, 1);
  EXPECT_EQ(vertices[3].degree_head, 2);
  EXPECT_EQ(vertices[3].degree_tail, 0);

  EXPECT_EQ(vertices[4].degree_undirected, 2);
  EXPECT_EQ(vertices[4].degree_head, 0);
  EXPECT_EQ(vertices[4].degree_tail, 0);
}

TEST(GraphTest, Clear) {
  Graph g = get_T3();
  g.clear();
  EXPECT_EQ(g.graph_hash, 0);
  EXPECT_EQ(g.edge_count, 0);
  EXPECT_EQ(g.undirected_edge_count, 0);
}

TEST(GraphTest, PermuteIsomorphic) {
  Graph g = get_T3();
  Graph h;
  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    EXPECT_EQ(g.graph_hash, h.graph_hash);
    EXPECT_TRUE(h.is_isomorphic(g));
    EXPECT_EQ(g.edge_count, 4);
    EXPECT_EQ(g.undirected_edge_count, 2);
  } while (std::next_permutation(p, p + 5));
}

TEST(GraphTest, PermuteCanonical) {
  Graph g = get_T3();
  VertexSignature vertices[MAX_VERTICES];
  g.canonicalize(vertices);
  Graph h;
  int p[5]{0, 1, 2, 3, 4};
  g.permute_canonical(p, h);
  EXPECT_TRUE(g.is_identical(h));
  EXPECT_EQ(h.edge_count, 4);
  EXPECT_EQ(h.undirected_edge_count, 2);
}

TEST(GraphTest, PermuteCanonical2) {
  Graph::set_global_graph_info(2, 4);
  Graph h, f;
  Graph g = parse_edges("{03, 12, 02>2, 13>3}");
  VertexSignature vertices[MAX_VERTICES];
  g.canonicalize(vertices);

  int p[4]{0, 1, 3, 2};
  g.permute_canonical(p, h);
  EXPECT_TRUE(g.is_isomorphic(h));
  h.permute_canonical(p, f);
  EXPECT_TRUE(g.is_identical(f));
}

TEST(GraphTest, Canonicalize) {
  Graph g = get_T3();
  Graph h = get_T3();

  EXPECT_TRUE(g.is_canonical);
  EXPECT_TRUE(h.is_canonical);
  EXPECT_EQ(g.graph_hash, h.graph_hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  EXPECT_TRUE(g.is_isomorphic(h));
  EXPECT_TRUE(h.is_identical(g));
  EXPECT_TRUE(g.is_identical(h));

  // Canonicalization should be idempotent.
  VertexSignature vertices[MAX_VERTICES];
  h.canonicalize(vertices);
  EXPECT_TRUE(h.is_canonical);
  EXPECT_EQ(g.graph_hash, h.graph_hash);
  EXPECT_TRUE(h.is_isomorphic(g));

  for (int v = 0; v < 4; v++) {
    EXPECT_GE(vertices[v].get_hash(), vertices[v + 1].get_hash());
  }
}

TEST(GraphTest, Canonicalize2) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{235, 345>4, 245, 456>4}");
  VertexSignature vertices[MAX_VERTICES];
  g.compute_vertex_signature(vertices);
  g.compute_graph_hash(vertices);

  EXPECT_EQ(vertices[0].get_degrees(), 0);
  EXPECT_EQ(vertices[1].get_degrees(), 0);
  EXPECT_EQ(vertices[2].get_degrees(), 0x020000);

  Graph h = g;
  h.canonicalize(vertices);
  EXPECT_EQ(g.graph_hash, h.graph_hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  EXPECT_TRUE(h.is_canonical);

  Graph f = get_T3();
  Graph::set_global_graph_info(3, 7);
  f.canonicalize(vertices);
  EXPECT_EQ(h.graph_hash, f.graph_hash);
}

TEST(GraphTest, Canonicalize3) {
  Graph::set_global_graph_info(2, 7);
  Graph g, h;
  g.add_edge(Edge(0b0101, UNDIRECTED));
  g.copy(&h);

  VertexSignature vertices[MAX_VERTICES];
  h.canonicalize(vertices);
  EXPECT_TRUE(h.is_canonical);
}

TEST(GraphTest, Copy) {
  Graph g = get_T3();
  g.add_edge(Edge(0b0111, UNDIRECTED));

  VertexSignature vertices[MAX_VERTICES];
  g.compute_vertex_signature(vertices);
  g.compute_graph_hash(vertices);
  Graph h;
  g.copy(&h);
  h.compute_vertex_signature(vertices);
  h.compute_graph_hash(vertices);

  EXPECT_EQ(g.graph_hash, h.graph_hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  EXPECT_EQ(g.edge_count, h.edge_count);
  EXPECT_EQ(g.undirected_edge_count, 3);
  EXPECT_EQ(g.undirected_edge_count, h.undirected_edge_count);
}

TEST(GraphTest, NonIsomorphic) {
  Graph g = get_T3();

  Graph h;
  g.copy(&h);
  h.add_edge(Edge(0b10110, UNDIRECTED));  // 124
  VertexSignature vertices[MAX_VERTICES];
  h.compute_vertex_signature(vertices);
  h.compute_graph_hash(vertices);

  Graph f;
  g.copy(&f);
  f.add_edge(Edge(0b10110, 1));  // 124
  f.compute_vertex_signature(vertices);
  f.compute_graph_hash(vertices);

  EXPECT_NE(g.graph_hash, f.graph_hash);
  EXPECT_FALSE(f.is_isomorphic(g));
  EXPECT_NE(h.graph_hash, f.graph_hash);
  EXPECT_FALSE(f.is_isomorphic(h));
}

TEST(GraphTest, NonIsomorphicWithSameHash) {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{012>0, 013>1, 024, 134, 234}");
  Graph h = parse_edges("{012>1, 013>0, 024, 134, 234}");
  VertexSignature vg[MAX_VERTICES];
  g.canonicalize(vg);
  VertexSignature vh[MAX_VERTICES];
  h.canonicalize(vh);

  EXPECT_FALSE(g.is_isomorphic(h));
  EXPECT_FALSE(h.is_isomorphic(g));
  EXPECT_EQ(g.graph_hash, h.graph_hash);
}

TEST(GraphTest, IsomorphicNotIdentical) {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{013>3, 023, 123, 014, 024>4, 124}");
  Graph h = parse_edges("{013, 023>3, 123, 014>4, 024, 124}");
  VertexSignature vg[MAX_VERTICES];
  g.canonicalize(vg);
  VertexSignature vh[MAX_VERTICES];
  h.canonicalize(vh);

  EXPECT_FALSE(g.is_identical(h));
  EXPECT_FALSE(h.is_identical(g));
  EXPECT_TRUE(g.is_isomorphic(h));
  EXPECT_TRUE(h.is_isomorphic(g));
}

TEST(GraphTest, ContainsT3) {
  Graph g = get_T3();
  Graph h;

  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    EXPECT_TRUE(h.contains_Tk(p[0]));
    EXPECT_TRUE(h.contains_Tk(p[1]));
    EXPECT_FALSE(h.contains_Tk(p[2]));
    EXPECT_TRUE(h.contains_Tk(p[3]));
    EXPECT_TRUE(h.contains_Tk(p[4]));
  } while (std::next_permutation(p, p + 5));
}

TEST(GraphTest, ContainsT3_B) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>0, 013>3, 024>4, 025>0, 045>4, 145>5, 245>4, 345>4}");
  EXPECT_TRUE(g.contains_Tk(5));
  EXPECT_TRUE(g.contains_Tk(4));
  EXPECT_TRUE(g.contains_Tk(2));
  EXPECT_TRUE(g.contains_Tk(0));
  EXPECT_FALSE(g.contains_Tk(1));
  EXPECT_FALSE(g.contains_Tk(3));
  EXPECT_FALSE(g.contains_Tk(6));
}

TEST(GraphTest, ContainsT3_C) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{013>3, 035, 135>3}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(g.contains_Tk(0));
  }
}

TEST(GraphTest, ContainsT3_D) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>0, 013>3, 024>4, 125, 035, 135>3, 245>5, 345>4}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(g.contains_Tk(0));
  }
}

TEST(GraphTest, ContainsT3_E) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>1, 013>3, 024>0, 134>4, 015>1, 125, 235, 045>0, 345>5}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(g.contains_Tk(0));
  }
}

TEST(GraphTest, NotContainsT3) {
  Graph h;
  Graph g = parse_edges("{013, 123, 023}");
  VertexSignature vertices[MAX_VERTICES];
  g.compute_vertex_signature(vertices);
  g.compute_graph_hash(vertices);

  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    for (int i = 0; i < 5; i++) {
      EXPECT_FALSE(h.contains_Tk(i));
    }
  } while (std::next_permutation(p, p + 5));
}

TEST(GraphTest, ContainsT2) {
  Graph::set_global_graph_info(2, 5);
  Graph g = parse_edges("{01>0, 12>1, 03>3, 13>3, 04>4, 24>4, 34>4}");
  EXPECT_TRUE(g.contains_Tk(4));
  EXPECT_TRUE(g.contains_Tk(3));
  EXPECT_TRUE(g.contains_Tk(0));
  EXPECT_TRUE(g.contains_Tk(1));
  EXPECT_FALSE(g.contains_Tk(2));
}

TEST(GraphTest, Theta) {
  Graph g = get_T3();
  EXPECT_EQ(g.get_theta(), Fraction(4, 1));

  Graph::set_global_graph_info(2, 5);
  Graph h = parse_edges("{01>0, 12>1, 03>3, 13>3, 04>4, 24>4, 34>4}");
  EXPECT_EQ(h.get_theta(), Fraction(10, 7));

  Graph::set_global_graph_info(2, 5);
  Graph j = parse_edges("{}");
  EXPECT_EQ(j.get_theta(), Fraction(1E8, 1));

  Graph::set_global_graph_info(3, 4);
  Graph k = parse_edges("{012>2, 123}");
  EXPECT_EQ(k.get_theta(), Fraction(3, 1));

  Graph l = parse_edges("{012>2, 123>1, 023>2, 013>0}");
  EXPECT_EQ(l.get_theta(), Fraction(1, 1));
}
