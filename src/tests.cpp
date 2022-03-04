#include "gmock/gmock.h"
#include "graph.h"
#include "graph.hpp"
#include "grower.h"
#include "gtest/gtest.h"

using namespace testing;

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

TEST(GraphTest, PermuteIsomorphic) {
  Graph<3, 5, 5> g = get_T3();
  Graph<3, 5, 5> h;
  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    EXPECT_EQ(g.hash, h.hash);
    EXPECT_TRUE(h.is_isomorphic(g));
  } while (next_permutation(p, p + 5));
}

TEST(GraphTest, PermuteCanonical) {
  Graph<3, 5, 5> g = get_T3();
  g.canonicalize();
  Graph<3, 5, 5> h;
  int p[5]{0, 1, 2, 3, 4};
  g.permute_canonical(p, h);
  EXPECT_TRUE(g.is_identical(h));
}

TEST(GraphTest, PermuteCanonical2) {
  Graph<2, 4, 4> g, h, f;
  g.add_edge(0b1001, UNDIRECTED);  // 03
  g.add_edge(0b0110, UNDIRECTED);  // 12
  g.add_edge(0b0101, 2);           // 02>2
  g.add_edge(0b1010, 3);           // 13>3
  g.init();
  g.canonicalize();

  int p[4]{0, 1, 3, 2};
  g.permute_canonical(p, h);
  EXPECT_TRUE(g.is_isomorphic(h));
  h.permute_canonical(p, f);
  EXPECT_TRUE(g.is_identical(f));
}

TEST(GraphTest, Canonicalize) {
  Graph<3, 5, 5> g = get_T3();
  Graph<3, 5, 5> h = get_T3();
  h.canonicalize();

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

  Graph<3, 8, 5> h = g;
  h.canonicalize();
  EXPECT_EQ(g.hash, h.hash);
  EXPECT_TRUE(h.is_isomorphic(g));
  EXPECT_TRUE(h.is_canonical);
  EXPECT_EQ(h.vertex_count, 5);

  Graph<3, 5, 5> f = get_T3();
  EXPECT_EQ(h.hash, f.hash);
}

TEST(GraphTest, Canonicalize3) {
  Graph<2, 7, 21> g, h;
  g.add_edge(0b0101, UNDIRECTED);
  g.init();
  g.copy(h);
  h.canonicalize();
  EXPECT_TRUE(h.is_canonical);
  EXPECT_EQ(h.vertex_count, 2);
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

TEST(GraphTest, ContainsT3) {
  Graph<3, 5, 5> g = get_T3();
  Graph<3, 5, 5> h;

  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    EXPECT_TRUE(h.contains_Tk(p[0]));
    EXPECT_TRUE(h.contains_Tk(p[1]));
    EXPECT_TRUE(h.contains_Tk(p[2]));
    EXPECT_TRUE(h.contains_Tk(p[3]));
    EXPECT_FALSE(h.contains_Tk(p[4]));
  } while (next_permutation(p, p + 5));
}

TEST(GraphTest, NotContainsT3) {
  Graph<3, 5, 5> g, h;
  g.add_edge(0b1011, UNDIRECTED);  // 013
  g.add_edge(0b1110, UNDIRECTED);  // 123
  g.add_edge(0b1101, UNDIRECTED);  // 023
  g.init();

  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute(p, h);
    for (int i = 0; i < 5; i++) {
      EXPECT_FALSE(h.contains_Tk(i));
    }
  } while (next_permutation(p, p + 5));
}

TEST(GrowerTest, G72) {
  Grower<2, 7, 21> s;
  s.grow(3);
  EXPECT_EQ(s.canonicals[0].size(), 0);
  EXPECT_EQ(s.canonicals[1].size(), 1);
  EXPECT_EQ(s.canonicals[2].size(), 2);
  EXPECT_EQ(s.canonicals[3].size(), 13);
}

TEST(PermutatorTest, Permutate) {
  vector<pair<int, int>> sets{make_pair(0, 2)};
  Permutator<3> perm(move(sets));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2));
  EXPECT_FALSE(perm.next());
}

TEST(PermutatorTest, Permutate2) {
  vector<pair<int, int>> sets{make_pair(0, 2), make_pair(3, 5)};
  Permutator<6> perm(move(sets));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2, 3, 4, 5));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(0, 1, 2, 4, 3, 5));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2, 4, 3, 5));
  EXPECT_FALSE(perm.next());
}

TEST(PermutatorTest, Permutate3) {
  vector<pair<int, int>> sets{make_pair(0, 3), make_pair(3, 5)};
  Permutator<5> perm(move(sets));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(0, 1, 2, 4, 3));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(0, 2, 1, 3, 4));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(0, 2, 1, 4, 3));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2, 3, 4));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2, 4, 3));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 2, 0, 3, 4));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 2, 0, 4, 3));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(2, 0, 1, 3, 4));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(2, 0, 1, 4, 3));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(2, 1, 0, 3, 4));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(2, 1, 0, 4, 3));
  EXPECT_FALSE(perm.next());
}