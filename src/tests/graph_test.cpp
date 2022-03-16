#include "../graph.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "iso_stress_test.h"

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

  for (int codegree = 0; codegree <= 3; codegree++) {
    GraphInvariants gi;
    g.generate_graph_hash(gi, codegree);
    const VertexSignature* vertices = gi.vertices;

    EXPECT_EQ(vertices[0].deg.degree_undirected, 1);
    EXPECT_EQ(vertices[0].deg.degree_head, 0);
    EXPECT_EQ(vertices[0].deg.degree_tail, 0);

    EXPECT_EQ(vertices[1].deg.degree_undirected, 1);
    EXPECT_EQ(vertices[1].deg.degree_head, 0);
    EXPECT_EQ(vertices[1].deg.degree_tail, 2);

    EXPECT_EQ(vertices[2].deg.degree_undirected, 1);
    EXPECT_EQ(vertices[2].deg.degree_head, 1);
    EXPECT_EQ(vertices[2].deg.degree_tail, 0);

    EXPECT_EQ(vertices[3].deg.degree_undirected, 2);
    EXPECT_EQ(vertices[3].deg.degree_head, 0);
    EXPECT_EQ(vertices[3].deg.degree_tail, 1);
  }
}

// Utility function to create and initialize T_3.
Graph get_T3(int codegree = 0) {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{013, 123>2, 023, 234>2}");
  GraphInvariants gi;
  g.canonicalize(gi, codegree);
  return g;
}

TEST(GraphTest, T3) {
  Graph g = get_T3(0);

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

  GraphInvariants gi;
  for (int codegree = 0; codegree <= 3; codegree++) {
    g.generate_graph_hash(gi, codegree);
    const VertexSignature* vertices = gi.vertices;

    EXPECT_EQ(vertices[0].deg.degree_undirected, 2);
    EXPECT_EQ(vertices[0].deg.degree_head, 0);
    EXPECT_EQ(vertices[0].deg.degree_tail, 2);

    EXPECT_EQ(vertices[1].deg.degree_undirected, 1);
    EXPECT_EQ(vertices[1].deg.degree_head, 0);
    EXPECT_EQ(vertices[1].deg.degree_tail, 1);

    EXPECT_EQ(vertices[2].deg.degree_undirected, 0);
    EXPECT_EQ(vertices[2].deg.degree_head, 0);
    EXPECT_EQ(vertices[2].deg.degree_tail, 1);

    EXPECT_EQ(vertices[3].deg.degree_undirected, 1);
    EXPECT_EQ(vertices[3].deg.degree_head, 2);
    EXPECT_EQ(vertices[3].deg.degree_tail, 0);

    EXPECT_EQ(vertices[4].deg.degree_undirected, 2);
    EXPECT_EQ(vertices[4].deg.degree_head, 0);
    EXPECT_EQ(vertices[4].deg.degree_tail, 0);
  }
}

TEST(GraphTest, Clear) {
  Graph g = get_T3(0);
  g.clear();
  EXPECT_EQ(g.graph_hash, 0);
  EXPECT_EQ(g.edge_count, 0);
  EXPECT_EQ(g.undirected_edge_count, 0);
}

TEST(GraphTest, IsomorphicSlow) {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{013>3, 023>3, 014, 034}");
  g.finalize_edges();
  Graph h = parse_edges("{014>0, 034>0, 124, 024}");
  h.finalize_edges();
  EXPECT_TRUE(g.is_isomorphic_slow(h));
  EXPECT_TRUE(h.is_isomorphic_slow(g));
}

TEST(GraphTest, PermuteIsomorphic) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph g = get_T3(codegree);
    Graph h;
    int p[5]{0, 1, 2, 3, 4};
    GraphInvariants gi;
    do {
      g.permute(p, h, codegree);
      EXPECT_TRUE(g.is_isomorphic_slow(h));
      EXPECT_TRUE(h.is_isomorphic_slow(g));

      EXPECT_EQ(g.graph_hash, h.graph_hash);
      h.canonicalize(gi, codegree);
      EXPECT_TRUE(h.is_isomorphic(g));
      EXPECT_EQ(g.edge_count, 4);
      EXPECT_EQ(g.undirected_edge_count, 2);
    } while (std::next_permutation(p, p + 5));
  }
}

TEST(GraphTest, PermuteCanonical) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph g = get_T3(codegree);
    GraphInvariants gi;
    g.canonicalize(gi, 0);
    Graph h;
    int p[5]{0, 1, 2, 3, 4};
    g.permute_canonical(p, h);
    EXPECT_TRUE(g.is_identical(h));
    EXPECT_EQ(h.edge_count, 4);
    EXPECT_EQ(h.undirected_edge_count, 2);
  }
}

TEST(GraphTest, PermuteCanonical2) {
  for (int codegree = 0; codegree <= 2; codegree++) {
    Graph::set_global_graph_info(2, 4);
    Graph h, f;
    Graph g = parse_edges("{03, 12, 02>2, 13>3}");
    GraphInvariants gi;
    g.canonicalize(gi, codegree);

    int p[4]{0, 1, 3, 2};
    g.permute_canonical(p, h);
    EXPECT_TRUE(g.is_isomorphic(h));
    h.permute_canonical(p, f);
    EXPECT_TRUE(g.is_identical(f));
  }
}

TEST(GraphTest, Canonicalize) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph g = get_T3(codegree);
    Graph h = get_T3(codegree);

    EXPECT_TRUE(g.is_canonical);
    EXPECT_TRUE(h.is_canonical);
    EXPECT_EQ(g.graph_hash, h.graph_hash);
    EXPECT_TRUE(h.is_isomorphic(g));
    EXPECT_TRUE(g.is_isomorphic(h));
    EXPECT_TRUE(h.is_identical(g));
    EXPECT_TRUE(g.is_identical(h));

    // Canonicalization should be idempotent.
    GraphInvariants gi;
    h.canonicalize(gi, codegree);
    EXPECT_TRUE(h.is_canonical);
    EXPECT_EQ(g.graph_hash, h.graph_hash);
    EXPECT_TRUE(h.is_isomorphic(g));

    for (int v = 0; v < 4; v++) {
      EXPECT_GE(gi.vertices[v].get_hash(), gi.vertices[v + 1].get_hash());
    }
  }
}

TEST(GraphTest, Canonicalize2) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph::set_global_graph_info(3, 7);
    Graph g = parse_edges("{235, 345>4, 245, 456>4}");
    GraphInvariants gi;
    g.generate_graph_hash(gi, codegree);

    EXPECT_EQ(gi.vertices[0].deg.get_degrees(), 0);
    EXPECT_EQ(gi.vertices[1].deg.get_degrees(), 0);
    EXPECT_EQ(gi.vertices[2].deg.get_degrees(), 0x02);

    Graph h = g;
    h.canonicalize(gi, codegree);
    EXPECT_EQ(g.graph_hash, h.graph_hash);
    EXPECT_TRUE(h.is_canonical);

    Graph f = get_T3(codegree);
    Graph::set_global_graph_info(3, 7);
    f.canonicalize(gi, codegree);
    EXPECT_EQ(h.graph_hash, f.graph_hash);
  }
}

TEST(GraphTest, Canonicalize3) {
  Graph::set_global_graph_info(2, 7);
  Graph g, h;
  g.add_edge(Edge(0b0101, UNDIRECTED));
  g.copy(&h);

  GraphInvariants gi;
  h.canonicalize(gi, 0);
  EXPECT_TRUE(h.is_canonical);
}

TEST(GraphTest, Codegrees3) {
  Graph g = get_T3(0);
  Graph h = get_T3(2);
  Graph f = get_T3(3);
  EXPECT_NE(g.graph_hash, h.graph_hash);
  EXPECT_NE(g.graph_hash, f.graph_hash);
  EXPECT_NE(f.graph_hash, h.graph_hash);
}

Graph get_G4(int codegree) {
  Graph::set_global_graph_info(4, 7);
  Graph g = parse_edges(
      "{0125>5, 0135>5, 0235>5, 0145>5, 1245>1, 0345>5, 2345, 0126>6, 0136>6, 1236>6, 0146>6, "
      "0246>6, 1246>6, 0346>6, 2356>2}");
  GraphInvariants gi;
  g.canonicalize(gi, codegree);
  return g;
}

TEST(GraphTest, Codegrees4) {
  Graph g0 = get_G4(0);
  Graph g2 = get_G4(2);
  Graph g3 = get_G4(3);
  EXPECT_NE(g0.graph_hash, g2.graph_hash);
  EXPECT_NE(g0.graph_hash, g3.graph_hash);
  EXPECT_NE(g2.graph_hash, g3.graph_hash);
}

TEST(GraphTest, Copy) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph g = get_T3(codegree);
    g.add_edge(Edge(0b0111, UNDIRECTED));

    GraphInvariants gi;
    g.generate_graph_hash(gi, codegree);
    Graph h;
    g.copy(&h);
    h.generate_graph_hash(gi, codegree);

    EXPECT_EQ(g.graph_hash, h.graph_hash);
    EXPECT_TRUE(h.is_isomorphic(g));
    EXPECT_EQ(g.edge_count, h.edge_count);
    EXPECT_EQ(g.undirected_edge_count, 3);
    EXPECT_EQ(g.undirected_edge_count, h.undirected_edge_count);
  }
}

TEST(GraphTest, NonIsomorphic) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph g = get_T3(codegree);

    Graph h;
    g.copy(&h);
    h.add_edge(Edge(0b10110, UNDIRECTED));  // 124
    GraphInvariants gi;
    h.generate_graph_hash(gi, codegree);

    Graph f;
    g.copy(&f);
    f.add_edge(Edge(0b10110, 1));  // 124
    f.generate_graph_hash(gi, codegree);

    EXPECT_NE(g.graph_hash, f.graph_hash);
    EXPECT_FALSE(f.is_isomorphic(g));
    EXPECT_NE(h.graph_hash, f.graph_hash);
    EXPECT_FALSE(f.is_isomorphic(h));
  }
}

TEST(GraphTest, NonIsomorphicWithSameHash) {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{012>0, 013>1, 024, 134, 234}");
  Graph h = parse_edges("{012>1, 013>0, 024, 134, 234}");
  GraphInvariants gi, hi;

  // When not using codegree info, the two graphs have the same hash.
  g.canonicalize(gi, 0);
  h.canonicalize(hi, 0);
  EXPECT_FALSE(g.is_isomorphic(h));
  EXPECT_FALSE(h.is_isomorphic(g));
  EXPECT_EQ(g.graph_hash, h.graph_hash);

  // When using codegree info, the two graphs have different hashes.
  g.canonicalize(gi, 2);
  h.canonicalize(hi, 2);
  EXPECT_NE(g.graph_hash, h.graph_hash);
  EXPECT_FALSE(g.is_isomorphic(h));
  EXPECT_FALSE(h.is_isomorphic(g));
}

TEST(GraphTest, IsomorphicWithSameHash) {
  Graph::set_global_graph_info(2, 6);
  Graph g = parse_edges("{02, 12>1, 04>0, 05>5, 15>5, 35>5}");
  Graph h = parse_edges("{02, 12>1, 03>0, 05>5, 15>5, 45>5}");
  GraphInvariants gi, hi;
  g.canonicalize(gi, 0);
  h.canonicalize(hi, 0);
  EXPECT_EQ(g.graph_hash, h.graph_hash);
  EXPECT_TRUE(g.is_identical(h));
  EXPECT_TRUE(h.is_isomorphic(g));
}

TEST(GraphTest, IsomorphicNotIdentical) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph::set_global_graph_info(3, 5);
    Graph g = parse_edges("{013>3, 023, 123, 014, 024>4, 124}");
    Graph h = parse_edges("{013, 023>3, 123, 014>4, 024, 124}");
    GraphInvariants gi, hi;
    g.canonicalize(gi, codegree);
    h.canonicalize(hi, codegree);

    EXPECT_FALSE(g.is_identical(h));
    EXPECT_FALSE(h.is_identical(g));
    EXPECT_TRUE(g.is_isomorphic(h));
    EXPECT_TRUE(h.is_isomorphic(g));
  }
}

TEST(GraphTest, Isomorphic_B) {
  Graph::set_global_graph_info(2, 3);
  Graph g = parse_edges("{01>0, 02>2, 12>1}");
  Graph h = parse_edges("{01>1, 02>0, 12>2}");
  EXPECT_TRUE(g.is_isomorphic_slow(h));
}

TEST(GraphTest, Isomorphic_C) {
  Graph::set_global_graph_info(5, 6);
  Graph g = parse_edges("{01234>4, 01245>5, 01345>4}");
  Graph h = parse_edges("{01234>4, 01245>4, 01345>5}");
  g.finalize_edges();
  h.finalize_edges();
  EXPECT_TRUE(g.is_isomorphic_slow(h));
  EXPECT_TRUE(h.is_isomorphic_slow(g));

  GraphInvariants gi;
  g.canonicalize(gi, 0);
  h.canonicalize(gi, 0);
  EXPECT_TRUE(g.is_isomorphic(h));
  EXPECT_TRUE(h.is_isomorphic(g));
}

TEST(GraphTest, ContainsT3) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph g = get_T3(codegree);
    Graph h;

    int p[5]{0, 1, 2, 3, 4};
    do {
      g.permute(p, h, codegree);
      EXPECT_TRUE(h.contains_Tk(p[0]));
      EXPECT_TRUE(h.contains_Tk(p[1]));
      EXPECT_FALSE(h.contains_Tk(p[2]));
      EXPECT_TRUE(h.contains_Tk(p[3]));
      EXPECT_TRUE(h.contains_Tk(p[4]));
    } while (std::next_permutation(p, p + 5));
  }
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

TEST(GraphTest, ContainsT3_F) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{013>3, 014>4, 024>4, 124, 234>4, 135>5, 045>4, 245>4, 345>4}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(g.contains_Tk(0));
  }
}

TEST(GraphTest, ContainsT3_G) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>1, 013>3, 134, 125>1, 135>5, 235>5, 045>4, 245>4}");
  EXPECT_TRUE(g.contains_Tk(1));
  EXPECT_TRUE(g.contains_Tk(2));
  EXPECT_TRUE(g.contains_Tk(3));
  EXPECT_TRUE(g.contains_Tk(5));
  EXPECT_FALSE(g.contains_Tk(0));
  EXPECT_FALSE(g.contains_Tk(4));
  EXPECT_FALSE(g.contains_Tk(6));
}

TEST(GraphTest, ContainsT3_H) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>1, 013>3, 134, 125>1, 135, 235, 045>4, 245>4}");
  EXPECT_TRUE(g.contains_Tk(1));
  EXPECT_TRUE(g.contains_Tk(2));
  EXPECT_TRUE(g.contains_Tk(3));
  EXPECT_TRUE(g.contains_Tk(5));
  EXPECT_FALSE(g.contains_Tk(0));
  EXPECT_FALSE(g.contains_Tk(4));
  EXPECT_FALSE(g.contains_Tk(6));
}

TEST(GraphTest, NotContainsT3) {
  for (int codegree = 0; codegree <= 3; codegree++) {
    Graph h;
    Graph g = parse_edges("{013, 123, 023}");
    GraphInvariants gi;
    g.generate_graph_hash(gi, codegree);

    int p[5]{0, 1, 2, 3, 4};
    do {
      g.permute(p, h, codegree);
      for (int i = 0; i < 5; i++) {
        EXPECT_FALSE(h.contains_Tk(i));
      }
    } while (std::next_permutation(p, p + 5));
  }
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

TEST(GraphTest, IsomorphicStress) {
  for (int diff = 0; diff <= 3; diff++) {
    for (int n = diff + 2; n <= 4; n++) {
      int k = n - diff;
      for (int c = 0; c < k; c++) {
        if (c == 1) continue;
        IsomorphismStressTest t(k, n, c);
        t.run();
      }
    }
  }
}