#include "../edge_gen.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(EdgeGeneratorTest, Generate22) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(2, 2);

  // 3 edges: {01} {01>0} {01>1}
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b11);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i));
  }

  // The enumeration has completed.
  EXPECT_FALSE(edge_gen.next());

  // Make sure initialize cleans up everything and works again.
  edge_gen.initialize(2, 2);
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b11);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i));
  }
  EXPECT_FALSE(edge_gen.next());
}

TEST(EdgeGeneratorTest, Generate23) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(2, 3);

  // First 3: {02} {02>0} {02>2}
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b101);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i == 1 ? 2 : i));
  }

  // Next 3: {12} {12>1} {12>2}
  for (int i = 0; i <= 2; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b110);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i == 0 ? UNDIRECTED : i));
  }

  // Next 9: {02, 12} {02>0, 12} {02>2, 12}
  //         {02, 12>1} {02>0, 12>1} {02>2, 12>1}
  //         {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  for (int i1 = 0; i1 <= 2; i1++) {
    for (int i0 = -1; i0 <= 1; i0++) {
      EXPECT_TRUE(edge_gen.next());
      EXPECT_EQ(edge_gen.edge_count, 2);
      EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b101);
      EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i0 == 1 ? 2 : i0));
      EXPECT_EQ(edge_gen.edges[1].vertex_set, 0b110);
      EXPECT_EQ(edge_gen.edges[1].head_vertex, static_cast<uint8>(i1 == 0 ? UNDIRECTED : i1));
    }
  }

  EXPECT_FALSE(edge_gen.next());
}

TEST(EdgeGeneratorTest, Generate33) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(3, 3);

  // First 3: {012} {012>0} {012>1} {012>2}
  for (int i = -1; i <= 2; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b111);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i));
  }
}

TEST(EdgeGeneratorTest, Generate35) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(3, 5);

  int count = 0;
  while (edge_gen.next()) {
    ++count;
  }

  int expected = 0;
  expected += compute_binom(6, 1) * 4;
  expected += compute_binom(6, 2) * 4 * 4;
  expected += compute_binom(6, 3) * 4 * 4 * 4;
  expected += compute_binom(6, 4) * 4 * 4 * 4 * 4;
  expected += compute_binom(6, 5) * 4 * 4 * 4 * 4 * 4;
  expected += compute_binom(6, 6) * 4 * 4 * 4 * 4 * 4 * 4;
  EXPECT_EQ(count, expected);
}

TEST(EdgeGeneratorTest, Generate45) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(4, 5);

  // First 5: {0124} {0124>0} {0124>1} {0124>2} {0124>4}
  for (int i = -1; i <= 3; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b10111);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i == 3 ? 4 : i));
  }
  // Next 5: {0134} {0134>0} {0134>1} {0134>3} {0134>4}
  for (int i = -1; i <= 3; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b11011);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i >= 2 ? i + 1 : i));
  }

  // Next 25: {0124, 0134} {0124>0, 0134} {0124>1, 0134} {0124>2, 0134} {0124>4, 0134}
  // {0124, 0134>0} {0124>0, 0134>0} {0124>1, 0134>0} {0124>2, 0134>0} {0124>4, 0134>0}
  // ... {0124, 0134>4} {0124>0, 0134>4} {0124>1, 0134>4} {0124>2, 0134>4} {0124>4, 0134>4}
  for (int i1 = -1; i1 <= 3; i1++) {
    for (int i0 = -1; i0 <= 3; i0++) {
      EXPECT_TRUE(edge_gen.next());
      EXPECT_EQ(edge_gen.edge_count, 2);
      EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b10111);
      EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i0 == 3 ? 4 : i0));
      EXPECT_EQ(edge_gen.edges[1].vertex_set, 0b11011);
      EXPECT_EQ(edge_gen.edges[1].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));
    }
  }

  // Next 5: {0234} {0234>0} {0234>2} {0234>3} {0234>4}
  for (int i = -1; i <= 3; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b11101);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i >= 1 ? i + 1 : i));
  }
  // Next 50
  for (int t = 0; t < 50; t++) {
    EXPECT_TRUE(edge_gen.next());
  }
  // The first 3-edge group.
  EXPECT_TRUE(edge_gen.next());
  EXPECT_EQ(edge_gen.edge_count, 3);
  EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b10111);
  EXPECT_EQ(edge_gen.edges[0].head_vertex, UNDIRECTED);
  EXPECT_EQ(edge_gen.edges[1].vertex_set, 0b11011);
  EXPECT_EQ(edge_gen.edges[1].head_vertex, UNDIRECTED);
  EXPECT_EQ(edge_gen.edges[2].vertex_set, 0b11101);
  EXPECT_EQ(edge_gen.edges[2].head_vertex, UNDIRECTED);
  // Next 123
  for (int t = 0; t < 123; t++) {
    EXPECT_TRUE(edge_gen.next());
  }
  // The end of the 3-edge group.
  EXPECT_TRUE(edge_gen.next());
  EXPECT_EQ(edge_gen.edge_count, 3);
  EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b10111);
  EXPECT_EQ(edge_gen.edges[0].head_vertex, 4);
  EXPECT_EQ(edge_gen.edges[1].vertex_set, 0b11011);
  EXPECT_EQ(edge_gen.edges[1].head_vertex, 4);
  EXPECT_EQ(edge_gen.edges[2].vertex_set, 0b11101);
  EXPECT_EQ(edge_gen.edges[2].head_vertex, 4);
}