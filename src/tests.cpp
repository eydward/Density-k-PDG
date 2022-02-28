#include "graph.h"
#include "gtest/gtest.h"

TEST(GraphTest, Init) {
  Graph<3, 7, 10> g;
  g.add_edge(0b11100, UNDIRECTED);  // 234
  g.add_edge(0b1100010, 5);         // 156>5
  g.add_edge(0b1110, 2);            // 123>2
  g.add_edge(0b1011, UNDIRECTED);   // 013
  g.init();

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

TEST(GraphTest, T3) {
  Graph<3, 5, 5> g;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, 2);           // 123>2
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.add_edge(0b11100, 2);          // 234>2
  g.init();

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
  Graph<3, 5, 5> g;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, 2);           // 123>2
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.add_edge(0b11100, 2);          // 234>2
  g.init();

  g.clear();
  EXPECT_EQ(g.hash, 0);
  EXPECT_EQ(g.edge_count, 0);
  EXPECT_EQ(g.vertices[1].get_hash(), 0);
}

TEST(GraphTest, Permute) {
  Graph<3, 5, 5> g;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, 2);           // 123>2
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.add_edge(0b11100, 2);          // 234>2
  g.init();

  Graph<3, 5, 5> h;
  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    EXPECT_EQ(g.hash, h.hash);
    EXPECT_TRUE(h.is_isomorphic(g));

  } while (next_permutation(p, p + 5));
}

TEST(GraphTest, Canonicalize) {
  Graph<3, 5, 5> g;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, 2);           // 123>2
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.add_edge(0b11100, 2);          // 234>2
  g.init();

  Graph<3, 5, 5> h;
  g.canonicalize(h);

  EXPECT_EQ(g.hash, h.hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  for (int v = 0; v < 4; v++) {
    EXPECT_LE(h.vertices[v].get_hash(), h.vertices[v + 1].get_hash());
  }
}
