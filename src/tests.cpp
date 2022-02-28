#include "graph.h"
#include "gtest/gtest.h"

TEST(GraphTest, Init) {
  Graph<3, 7, 10> g;
  g.add_edge(0b11100, UNDIRECTED);  // 234
  g.add_edge(0b1100010, 5);         // 156>5
  g.add_edge(0b1110, 2);            // 123>2
  g.add_edge(0b1011, UNDIRECTED);   // 013
  g.init();

  EXPECT_FALSE(g.is_canonical);

  EXPECT_EQ(4, g.edge_count);
  EXPECT_EQ(g.edges[0].vertex_set, 0b1011);
  EXPECT_EQ(g.edges[0].head_vertex, UNDIRECTED);
  EXPECT_EQ(g.edges[1].vertex_set, 0b1110);
  EXPECT_EQ(g.edges[1].head_vertex, 2);
  EXPECT_EQ(g.edges[2].vertex_set, 0b11100);
  EXPECT_EQ(g.edges[2].head_vertex, UNDIRECTED);
  EXPECT_EQ(g.edges[3].vertex_set, 0b1100010);
  EXPECT_EQ(g.edges[3].head_vertex, 5);

  EXPECT_EQ(g.vertices[0].degree_undirected, 1);
  EXPECT_EQ(g.vertices[0].degree_head, 0);
  EXPECT_EQ(g.vertices[0].degree_tail, 0);

  EXPECT_EQ(g.vertices[1].degree_undirected, 1);
  EXPECT_EQ(g.vertices[1].degree_head, 0);
  EXPECT_EQ(g.vertices[1].degree_tail, 2);

  EXPECT_EQ(g.vertices[2].degree_undirected, 1);
  EXPECT_EQ(g.vertices[2].degree_head, 1);
  EXPECT_EQ(g.vertices[2].degree_tail, 0);

  EXPECT_EQ(g.vertices[3].degree_undirected, 2);
  EXPECT_EQ(g.vertices[3].degree_head, 0);
  EXPECT_EQ(g.vertices[3].degree_tail, 1);
}

// Utility function to create and initialize T_3.
Graph<3, 5, 5> get_T3() {
  Graph<3, 5, 5> g;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, 2);           // 123>2
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.add_edge(0b11100, 2);          // 234>2
  g.init();
  return g;
}

TEST(GraphTest, T3) {
  Graph<3, 5, 5> g = get_T3();

  EXPECT_EQ(4, g.edge_count);
  EXPECT_EQ(g.edges[0].vertex_set, 0b1011);
  EXPECT_EQ(g.edges[0].head_vertex, UNDIRECTED);
  EXPECT_EQ(g.edges[1].vertex_set, 0b1101);
  EXPECT_EQ(g.edges[1].head_vertex, UNDIRECTED);
  EXPECT_EQ(g.edges[2].vertex_set, 0b1110);
  EXPECT_EQ(g.edges[2].head_vertex, 2);
  EXPECT_EQ(g.edges[3].vertex_set, 0b11100);
  EXPECT_EQ(g.edges[3].head_vertex, 2);

  EXPECT_EQ(g.vertices[0].degree_undirected, 2);
  EXPECT_EQ(g.vertices[0].degree_head, 0);
  EXPECT_EQ(g.vertices[0].degree_tail, 0);

  EXPECT_EQ(g.vertices[1].degree_undirected, 1);
  EXPECT_EQ(g.vertices[1].degree_head, 0);
  EXPECT_EQ(g.vertices[1].degree_tail, 1);

  EXPECT_EQ(g.vertices[2].degree_undirected, 1);
  EXPECT_EQ(g.vertices[2].degree_head, 2);
  EXPECT_EQ(g.vertices[2].degree_tail, 0);

  EXPECT_EQ(g.vertices[3].degree_undirected, 2);
  EXPECT_EQ(g.vertices[3].degree_head, 0);
  EXPECT_EQ(g.vertices[3].degree_tail, 2);

  EXPECT_EQ(g.vertices[4].degree_undirected, 0);
  EXPECT_EQ(g.vertices[4].degree_head, 0);
  EXPECT_EQ(g.vertices[4].degree_tail, 1);
}

TEST(GraphTest, Clear) {
  Graph<3, 5, 5> g = get_T3();
  g.clear();
  EXPECT_EQ(g.hash, 0);
  EXPECT_EQ(g.edge_count, 0);
  EXPECT_EQ(g.vertices[1].get_hash(), 0);
}

TEST(GraphTest, Permute) {
  Graph<3, 5, 5> g = get_T3();
  Graph<3, 5, 5> h;
  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    EXPECT_EQ(g.hash, h.hash);
    EXPECT_TRUE(h.is_isomorphic(g));

  } while (next_permutation(p, p + 5));
}

TEST(GraphTest, Canonicalize) {
  Graph<3, 5, 5> g = get_T3();
  Graph<3, 5, 5> h;
  g.canonicalize(h);

  EXPECT_FALSE(g.is_canonical);
  EXPECT_TRUE(h.is_canonical);
  EXPECT_EQ(g.hash, h.hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  for (int v = 0; v < 4; v++) {
    EXPECT_GE(h.vertices[v].get_hash(), h.vertices[v + 1].get_hash());
  }
}

TEST(GraphTest, Canonicalize2) {
  Graph<3, 8, 5> g;
  g.add_edge(0b1011000, UNDIRECTED);  // 346
  g.add_edge(0b1110000, 5);           // 456>5
  g.add_edge(0b1101000, UNDIRECTED);  // 356
  g.add_edge(0b11100000, 5);          // 567>5
  g.init();
  EXPECT_EQ(g.vertices[0].get_degrees(), 0);
  EXPECT_EQ(g.vertices[1].get_degrees(), 0);
  EXPECT_EQ(g.vertices[2].get_degrees(), 0);

  Graph<3, 8, 5> h;
  g.canonicalize(h);
  EXPECT_EQ(g.hash, h.hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  EXPECT_TRUE(h.is_canonical);
  EXPECT_EQ(h.vertex_count, 5);

  Graph<3, 5, 5> f = get_T3();
  EXPECT_EQ(h.hash, f.hash);
}

TEST(GraphTest, Copy) {
  Graph<3, 5, 5> g = get_T3();
  Graph<3, 5, 5> h;
  g.copy_without_init(h);
  h.init();
  EXPECT_EQ(g.hash, h.hash);
  EXPECT_TRUE(h.is_isomorphic(g));
}

TEST(GraphTest, NonIsomorphic) {
  Graph<3, 5, 5> g = get_T3();

  Graph<3, 5, 5> h;
  g.copy_without_init(h);
  h.add_edge(0b10110, UNDIRECTED);  // 124
  h.init();

  Graph<3, 5, 5> f;
  g.copy_without_init(f);
  f.add_edge(0b10110, 1);  // 124
  f.init();

  EXPECT_NE(g.hash, f.hash);
  EXPECT_FALSE(f.is_isomorphic(g));
  EXPECT_NE(h.hash, f.hash);
  EXPECT_FALSE(f.is_isomorphic(h));
}
