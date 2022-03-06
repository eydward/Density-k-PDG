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

  // Make sure reset_enumeration cleans up everything and works again.
  edge_gen.reset_enumeration();
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b11);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i));
  }
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

  // Next 12: {12} {02, 12} {02>0, 12} {02>2, 12}
  //          {12>1} {02, 12>1} {02>0, 12>1} {02>2, 12>1}
  //          {12>2} {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  for (int i1 = 0; i1 <= 2; i1++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b110);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i1 == 0 ? UNDIRECTED : i1));

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

TEST(EdgeGeneratorTest, Generate23WithSkip) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(2, 3);

  // First 3: {02} {02>0} {02>2}
  for (int i = -1; i <= 1; i++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b101);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i == 1 ? 2 : i));
  }

  // Next: {12}
  EXPECT_TRUE(edge_gen.next());
  EXPECT_EQ(edge_gen.edge_count, 1);
  EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b110);
  EXPECT_EQ(edge_gen.edges[0].head_vertex, UNDIRECTED);

  // Notify to skip, so the next 3 are skipped: {02, 12} {02>0, 12} {02>2, 12}.
  edge_gen.notify_contain_tk_skip();

  // Next: {12>1}
  EXPECT_TRUE(edge_gen.next());
  EXPECT_EQ(edge_gen.edge_count, 1);
  EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b110);
  EXPECT_EQ(edge_gen.edges[0].head_vertex, 1);

  // Notify to skip, so the next 3 are skipped: {02, 12>1} {02>0, 12>1} {02>2, 12>1}.
  edge_gen.notify_contain_tk_skip();

  // Next 4: {12>2} {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  EXPECT_TRUE(edge_gen.next());
  EXPECT_EQ(edge_gen.edge_count, 1);
  EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b110);
  EXPECT_EQ(edge_gen.edges[0].head_vertex, 2);

  for (int i0 = -1; i0 <= 1; i0++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 2);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b101);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i0 == 1 ? 2 : i0));
    EXPECT_EQ(edge_gen.edges[1].vertex_set, 0b110);
    EXPECT_EQ(edge_gen.edges[1].head_vertex, 2);
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

  int expected = 5 * 5 * 5 * 5 * 5 * 5 - 1;
  EXPECT_EQ(count, expected);
}

TEST(EdgeGeneratorTest, Generate27) {
  EdgeGenerator edge_gen;
  edge_gen.initialize(2, 7);

  int count = 0;
  while (edge_gen.next()) {
    ++count;
  }

  int expected = 4 * 4 * 4 * 4 * 4 * 4 - 1;
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

  // Next 30:
  //   {0134} {0124, 0134} {0124>0, 0134} {0124>1, 0134} {0124>2, 0134} {0124>4, 0134}
  //   {0134>0} {0124, 0134>0} {0124>0, 0134>0} {0124>1, 0134>0} {0124>2, 0134>0} {0124>4, 0134>0}
  //   ...
  //   {0134>4} {0124, 0134>4} {0124>0, 0134>4} {0124>1, 0134>4} {0124>2, 0134>4} {0124>4, 0134>4}
  for (int i1 = -1; i1 <= 3; i1++) {
    EXPECT_TRUE(edge_gen.next());
    EXPECT_EQ(edge_gen.edge_count, 1);
    EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b11011);
    EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));

    for (int i0 = -1; i0 <= 3; i0++) {
      EXPECT_TRUE(edge_gen.next());
      EXPECT_EQ(edge_gen.edge_count, 2);
      EXPECT_EQ(edge_gen.edges[0].vertex_set, 0b10111);
      EXPECT_EQ(edge_gen.edges[0].head_vertex, static_cast<uint8>(i0 == 3 ? 4 : i0));
      EXPECT_EQ(edge_gen.edges[1].vertex_set, 0b11011);
      EXPECT_EQ(edge_gen.edges[1].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));
    }
  }
}