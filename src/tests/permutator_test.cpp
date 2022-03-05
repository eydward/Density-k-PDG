#include "../permutator.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(PermutatorTest, Permutate) {
  std::vector<std::pair<int, int>> sets{std::make_pair(0, 2)};
  Permutator<3> perm(std::move(sets));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2));
  EXPECT_FALSE(perm.next());
}

TEST(PermutatorTest, Permutate2) {
  std::vector<std::pair<int, int>> sets{std::make_pair(0, 2), std::make_pair(3, 5)};
  Permutator<6> perm(std::move(sets));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2, 3, 4, 5));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(0, 1, 2, 4, 3, 5));
  EXPECT_TRUE(perm.next());
  ASSERT_THAT(perm.p, ElementsAre(1, 0, 2, 4, 3, 5));
  EXPECT_FALSE(perm.next());
}

TEST(PermutatorTest, Permutate3) {
  std::vector<std::pair<int, int>> sets{std::make_pair(0, 3), std::make_pair(3, 5)};
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