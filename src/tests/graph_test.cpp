#include "../graph.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(GraphTest, Init) {
  Graph::set_global_graph_info(3, 7);
  Graph g;
  g.add_edge(Edge(0b11100, UNDIRECTED));  // 234
  g.add_edge(Edge(0b1100010, 5));         // 156>5
  g.add_edge(Edge(0b1110, 2));            // 123>2
  g.add_edge(Edge(0b1011, UNDIRECTED));   // 013

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
  Graph g;
  g.add_edge(Edge(0b1011, UNDIRECTED));  // 013
  g.add_edge(Edge(0b1110, 2));           // 123>2
  g.add_edge(Edge(0b1101, UNDIRECTED));  // 023
  g.add_edge(Edge(0b11100, 2));          // 234>2

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
  Graph g, h, f;
  g.add_edge(Edge(0b1001, UNDIRECTED));  // 03
  g.add_edge(Edge(0b0110, UNDIRECTED));  // 12
  g.add_edge(Edge(0b0101, 2));           // 02>2
  g.add_edge(Edge(0b1010, 3));           // 13>3
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
  Graph g;
  g.add_edge(Edge(0b0101100, UNDIRECTED));  // 235
  g.add_edge(Edge(0b0111000, 4));           // 345>4
  g.add_edge(Edge(0b0110100, UNDIRECTED));  // 245
  g.add_edge(Edge(0b1110000, 4));           // 456>4

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

TEST(GraphTest, NotContainsT3) {
  Graph g, h;
  g.add_edge(Edge(0b1011, UNDIRECTED));  // 013
  g.add_edge(Edge(0b1110, UNDIRECTED));  // 123
  g.add_edge(Edge(0b1101, UNDIRECTED));  // 023

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
  Graph g;
  g.add_edge(Edge(0b11, 0));     // 01>0
  g.add_edge(Edge(0b110, 1));    // 12>1
  g.add_edge(Edge(0b1001, 3));   // 03>3
  g.add_edge(Edge(0b1010, 3));   // 13>3
  g.add_edge(Edge(0b10001, 4));  // 04>4
  g.add_edge(Edge(0b10100, 4));  // 24>4
  g.add_edge(Edge(0b11000, 4));  // 34>4
  EXPECT_TRUE(g.contains_Tk(4));
  EXPECT_TRUE(g.contains_Tk(3));
  EXPECT_TRUE(g.contains_Tk(0));
  EXPECT_TRUE(g.contains_Tk(1));
  EXPECT_FALSE(g.contains_Tk(2));
}

TEST(GraphTest, Theta) {
  Graph g = get_T3();
  EXPECT_EQ(g.get_theta(), Fraction(4, 1));
}
