#include "../k4d3problem/graph_tk.h"

#include "../graph.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(GraphTkTest, ContainsT3) {
  Graph::set_global_graph_info(3, 5);
  Graph g = parse_edges("{013, 123>2, 023, 234>2}");
  Graph h;

  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute_for_testing(p, h);
    EXPECT_TRUE(contains_Tk(h, p[0]));
    EXPECT_TRUE(contains_Tk(h, p[3]));
    EXPECT_FALSE(contains_Tk(h, p[4]));
    EXPECT_TRUE(contains_Tk(h, p[2]));
    EXPECT_TRUE(contains_Tk(h, p[1]));
  } while (std::next_permutation(p, p + 5));
}

TEST(GraphTkTest, ContainsT3_B) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>0, 013>3, 024>4, 025>0, 045>4, 145>5, 245>4, 345>4}");
  EXPECT_TRUE(contains_Tk(g, 5));
  EXPECT_TRUE(contains_Tk(g, 4));
  EXPECT_TRUE(contains_Tk(g, 2));
  EXPECT_TRUE(contains_Tk(g, 0));
  EXPECT_FALSE(contains_Tk(g, 1));
  EXPECT_FALSE(contains_Tk(g, 3));
  EXPECT_FALSE(contains_Tk(g, 6));
}

TEST(GraphTkTest, ContainsT3_C) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{013>3, 035, 135>3}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(contains_Tk(g, 0));
  }
}

TEST(GraphTkTest, ContainsT3_D) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>0, 013>3, 024>4, 125, 035, 135>3, 245>5, 345>4}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(contains_Tk(g, 0));
  }
}

TEST(GraphTkTest, ContainsT3_E) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>1, 013>3, 024>0, 134>4, 015>1, 125, 235, 045>0, 345>5}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(contains_Tk(g, 0));
  }
}

TEST(GraphTkTest, ContainsT3_F) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{013>3, 014>4, 024>4, 124, 234>4, 135>5, 045>4, 245>4, 345>4}");
  for (int v = 0; v < 7; v++) {
    EXPECT_FALSE(contains_Tk(g, 0));
  }
}

TEST(GraphTkTest, ContainsT3_G) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>1, 013>3, 134, 125>1, 135>5, 235>5, 045>4, 245>4}");
  EXPECT_TRUE(contains_Tk(g, 1));
  EXPECT_TRUE(contains_Tk(g, 2));
  EXPECT_TRUE(contains_Tk(g, 3));
  EXPECT_TRUE(contains_Tk(g, 5));
  EXPECT_FALSE(contains_Tk(g, 0));
  EXPECT_FALSE(contains_Tk(g, 4));
  EXPECT_FALSE(contains_Tk(g, 6));
}

TEST(GraphTkTest, ContainsT3_H) {
  Graph::set_global_graph_info(3, 7);
  Graph g = parse_edges("{012>1, 013>3, 134, 125>1, 135, 235, 045>4, 245>4}");
  EXPECT_TRUE(contains_Tk(g, 1));
  EXPECT_TRUE(contains_Tk(g, 2));
  EXPECT_TRUE(contains_Tk(g, 3));
  EXPECT_TRUE(contains_Tk(g, 5));
  EXPECT_FALSE(contains_Tk(g, 0));
  EXPECT_FALSE(contains_Tk(g, 4));
  EXPECT_FALSE(contains_Tk(g, 6));
}

TEST(GraphTkTest, NotContainsT3) {
  Graph h;
  Graph g = parse_edges("{013, 123, 023}");
  g.canonicalize();

  int p[5]{0, 1, 2, 3, 4};
  do {
    g.permute_for_testing(p, h);
    for (int i = 0; i < 5; i++) {
      EXPECT_FALSE(contains_Tk(h, i));
    }
  } while (std::next_permutation(p, p + 5));
}

TEST(GraphTkTest, ContainsT2) {
  Graph::set_global_graph_info(2, 5);
  Graph g = parse_edges("{01>0, 12>1, 03>3, 13>3, 04>4, 24>4, 34>4}");
  EXPECT_TRUE(contains_Tk(g, 4));
  EXPECT_TRUE(contains_Tk(g, 3));
  EXPECT_TRUE(contains_Tk(g, 0));
  EXPECT_TRUE(contains_Tk(g, 1));
  EXPECT_FALSE(contains_Tk(g, 2));
}

TEST(GraphTkTest, VertexCount12) {
  Graph::set_global_graph_info(2, 12);
  Graph g = parse_edges("{01, 23, 45, 67, 89, 9a, 9b, ab>a}");
  for (int v = 9; v < 12; v++) {
    EXPECT_TRUE(g.contains_Tk(v));
  }
  for (int v = 0; v < 9; v++) {
    EXPECT_FALSE(g.contains_Tk(v));
  }
}
