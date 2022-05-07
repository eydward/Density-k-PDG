#include "../graph.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(GraphK4D3Test, K4) {
  Graph::set_global_graph_info(2, 4);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12>2, 13>3, 23>3}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01>0, 02, 03>0, 12, 13>3, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03>0, 12, 13>3, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12, 13>3, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12, 13, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_TRUE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 13, 23}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_FALSE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12>1, 13>3, 23>2}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_FALSE(g.contains_K4D3(v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02>2, 03, 12>2, 13>3, 23>3}", g));
  for (int v = 0; v < 4; v++) {
    EXPECT_FALSE(g.contains_K4D3(v));
  }
}
