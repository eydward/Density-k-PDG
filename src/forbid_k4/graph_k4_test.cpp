#include "../forbid_k4/graph_k4.h"

#include "../graph.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(GraphK4Test, K3) {
  Graph::set_global_graph_info(2, 3);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{01, 02>0, 12>1}", g));
  for (int v = 0; v <= 2; v++) {
    EXPECT_FALSE(contains_K4(g, v));
  }
}

TEST(GraphK4Test, K4Undirected) {
  Graph::set_global_graph_info(2, 4);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12, 13, 23}", g));
  for (int v = 0; v <= 3; v++) {
    EXPECT_FALSE(contains_K4(g, v));
  }
}

TEST(GraphK4Test, K4Directed) {
  Graph::set_global_graph_info(2, 4);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{01>1, 02, 03, 12, 13, 23}", g));
  for (int v = 0; v <= 3; v++) {
    EXPECT_TRUE(contains_K4(g, v));
  }
}

TEST(GraphK4Test, K4MoreDirected) {
  Graph::set_global_graph_info(2, 4);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{01>1, 02, 03, 12>1, 13, 23}", g));
  for (int v = 0; v <= 3; v++) {
    EXPECT_TRUE(contains_K4(g, v));
  }
}

TEST(GraphK4Test, K4Mixed) {
  Graph::set_global_graph_info(2, 7);
  Graph g;
  ASSERT_TRUE(Graph::parse_edges("{01>1, 02, 03, 12>1, 13, 23, 14>1, 25>5, 36>3}", g));
  for (int v = 0; v <= 3; v++) {
    EXPECT_TRUE(contains_K4(g, v));
  }
  for (int v = 4; v <= 6; v++) {
    EXPECT_FALSE(contains_K4(g, v));
  }

  ASSERT_TRUE(Graph::parse_edges("{01, 02, 03, 12, 13, 23, 14>1, 25>5, 36>3}", g));
  for (int v = 0; v <= 6; v++) {
    EXPECT_FALSE(contains_K4(g, v));
  }
}
