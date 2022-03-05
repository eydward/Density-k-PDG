#include "../grower.hpp"

#include "../graph.hpp"
#include "gtest/gtest.h"

using namespace testing;

TEST(GrowerTest, G72) {
  Grower<2, 3> s;
  s.grow();
  EXPECT_EQ(s.canonicals[0].size(), 0);
  EXPECT_EQ(s.canonicals[1].size(), 1);
  EXPECT_EQ(s.canonicals[2].size(), 2);
  EXPECT_EQ(s.canonicals[3].size(), 7);
}

TEST(GrowerTest, G32) {
  Counters::initialize();
  Grower<2, 3> s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}
TEST(GrowerTest, G42) {
  Counters::initialize();
  Grower<2, 4> s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}
TEST(GrowerTest, G52) {
  Counters::initialize();
  Grower<2, 5> s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
}
TEST(GrowerTest, G43) {
  Counters::initialize();
  Grower<3, 4> s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}
