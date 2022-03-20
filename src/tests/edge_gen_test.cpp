#include "../edge_gen.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(EdgeGeneratorTest, Generate22) {
  Graph::set_global_graph_info(2, 2);
  EdgeGenerator edge_gen(2, Graph());
  Graph copy;

  // 3 edges: {01} {01>0} {01>1}
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b11);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i));
  }

  // The enumeration has completed.
  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate23) {
  Graph::set_global_graph_info(2, 3);
  EdgeGenerator edge_gen(3, Graph());
  Graph copy;

  // First 3: {02} {02>0} {02>2}
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b101);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i == 1 ? 2 : i));
  }

  // Next 12: {12} {02, 12} {02>0, 12} {02>2, 12}
  //          {12>1} {02, 12>1} {02>0, 12>1} {02>2, 12>1}
  //          {12>2} {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  for (int i1 = 0; i1 <= 2; i1++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i1 == 0 ? UNDIRECTED : i1));

    for (int i0 = -1; i0 <= 1; i0++) {
      EXPECT_TRUE(edge_gen.next(copy));
      EXPECT_EQ(copy.edge_count, 2);
      EXPECT_EQ(copy.edges[0].vertex_set, 0b101);
      EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i0 == 1 ? 2 : i0));
      EXPECT_EQ(copy.edges[1].vertex_set, 0b110);
      EXPECT_EQ(copy.edges[1].head_vertex, static_cast<uint8>(i1 == 0 ? UNDIRECTED : i1));
    }
  }

  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate23WithSkip) {
  Graph::set_global_graph_info(2, 3);
  EdgeGenerator edge_gen(3, Graph());
  Graph copy;

  // First 3: {02} {02>0} {02>2}
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b101);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i == 1 ? 2 : i));
  }

  // Next: {12}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.edge_count, 1);
  EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
  EXPECT_EQ(copy.edges[0].head_vertex, UNDIRECTED);

  // Notify to skip, so the next 3 are skipped: {02, 12} {02>0, 12} {02>2, 12}.
  edge_gen.notify_contain_tk_skip();

  // Next: {12>1}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.edge_count, 1);
  EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
  EXPECT_EQ(copy.edges[0].head_vertex, 1);

  // Notify to skip, so the next 3 are skipped: {02, 12>1} {02>0, 12>1} {02>2, 12>1}.
  edge_gen.notify_contain_tk_skip();

  // Next 4: {12>2} {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.edge_count, 1);
  EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
  EXPECT_EQ(copy.edges[0].head_vertex, 2);

  for (int i0 = -1; i0 <= 1; i0++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 2);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b101);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i0 == 1 ? 2 : i0));
    EXPECT_EQ(copy.edges[1].vertex_set, 0b110);
    EXPECT_EQ(copy.edges[1].head_vertex, 2);
  }

  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate33) {
  Graph::set_global_graph_info(3, 3);
  EdgeGenerator edge_gen(3, Graph());
  Graph copy;

  // First 3: {012} {012>0} {012>1} {012>2}
  for (int i = -1; i <= 2; i++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b111);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i));
  }
}

TEST(EdgeGeneratorTest, Generate35) {
  Graph::set_global_graph_info(3, 5);
  EdgeGenerator edge_gen(5, Graph());
  Graph copy;

  int count = 0;
  while (edge_gen.next(copy)) {
    ++count;
  }

  int expected = 5 * 5 * 5 * 5 * 5 * 5 - 1;
  EXPECT_EQ(count, expected);
}

TEST(EdgeGeneratorTest, Generate27) {
  Graph::set_global_graph_info(2, 7);
  EdgeGenerator edge_gen(7, Graph());
  Graph copy;

  int count = 0;
  while (edge_gen.next(copy)) {
    ++count;
  }

  int expected = 4 * 4 * 4 * 4 * 4 * 4 - 1;
  EXPECT_EQ(count, expected);
}

TEST(EdgeGeneratorTest, Generate45) {
  Graph::set_global_graph_info(4, 5);
  EdgeGenerator edge_gen(5, Graph());
  Graph copy;

  // First 5: {0124} {0124>0} {0124>1} {0124>2} {0124>4}
  for (int i = -1; i <= 3; i++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b10111);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i == 3 ? 4 : i));
  }

  // Next 30:
  //   {0134} {0124, 0134} {0124>0, 0134} {0124>1, 0134} {0124>2, 0134} {0124>4, 0134}
  //   {0134>0} {0124, 0134>0} {0124>0, 0134>0} {0124>1, 0134>0} {0124>2, 0134>0} {0124>4, 0134>0}
  //   ...
  //   {0134>4} {0124, 0134>4} {0124>0, 0134>4} {0124>1, 0134>4} {0124>2, 0134>4} {0124>4, 0134>4}
  for (int i1 = -1; i1 <= 3; i1++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b11011);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));

    for (int i0 = -1; i0 <= 3; i0++) {
      EXPECT_TRUE(edge_gen.next(copy));
      EXPECT_EQ(copy.edge_count, 2);
      EXPECT_EQ(copy.edges[0].vertex_set, 0b10111);
      EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i0 == 3 ? 4 : i0));
      EXPECT_EQ(copy.edges[1].vertex_set, 0b11011);
      EXPECT_EQ(copy.edges[1].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));
    }
  }
}