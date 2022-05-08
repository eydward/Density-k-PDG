#include "../graph_k4d3.h"

#include "gtest/gtest.h"

using namespace testing;

TEST(GraphK4D3Test, K4) {
  Graph::set_global_graph_info(2, 4);
  Graph g;

  EXPECT_EQ(g.get_zeta_ratio(), Fraction::infinity());

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12>2, 13>3, 23>3}", g));
  EXPECT_EQ(g.get_zeta_ratio(), Fraction(1, 1));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01>0, 02, 03>0, 12, 13>3, 23}", g));
  EXPECT_EQ(g.get_zeta_ratio(), Fraction(1, 1));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03>0, 12, 13>3, 23}", g));
  EXPECT_EQ(g.get_zeta_ratio(), Fraction(1, 1));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12, 13>3, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12, 13, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 13, 23}", g));
  EXPECT_EQ(g.get_zeta_ratio(), Fraction(6, 5));
  for (int v = 0; v < 4; v++) {
    EXPECT_FALSE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12>1, 13>3, 23>2}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_FALSE(contains_K4D3(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02>2, 03, 12>2, 13>3, 23>3}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_FALSE(contains_K4D3(g, v));
  }
}

TEST(GraphK4D3Test, K5) {
  Graph::set_global_graph_info(2, 5);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{41, 42, 43, 12>2, 13>3, 23>3}", g));
  EXPECT_EQ(g.get_zeta_ratio(), Fraction(7, 3));
  for (int v = 1; v < 5; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }
  EXPECT_FALSE(contains_K4D3(g, 0));

  ASSERT_TRUE(Graph::parse_edges("{04, 41, 42, 43, 12>2, 13>3, 23>3}", g));
  EXPECT_EQ(g.get_zeta_ratio(), Fraction(7, 4));
  for (int v = 1; v < 5; v++) {
    EXPECT_TRUE(contains_K4D3(g, v));
  }
  EXPECT_FALSE(contains_K4D3(g, 0));
}