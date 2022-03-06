#include "../grower.hpp"

#include "../graph.hpp"
#include "gtest/gtest.h"

using namespace testing;

TEST(GrowerTest, G27) {
  Grower<2, 3> s;
  s.grow(false);
  EXPECT_EQ(s.canonicals[0].size(), 0);
  EXPECT_EQ(s.canonicals[1].size(), 1);
  EXPECT_EQ(s.canonicals[2].size(), 2);
  EXPECT_EQ(s.canonicals[3].size(), 7);
}

TEST(GrowerTest, G23) {
  Counters::initialize();
  Grower<2, 3> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}

TEST(GrowerTest, G24) {
  Counters::initialize();
  Grower<2, 4> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}

TEST(GrowerTest, G25) {
  Counters::initialize();
  Grower<2, 5> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
}

TEST(GrowerTest, G26) {
  Counters::initialize();
  Grower<2, 6> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
}

TEST(GrowerTest, G34) {
  Counters::initialize();
  Grower<3, 4> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(4, 3));
}

TEST(GrowerTest, G45) {
  Counters::initialize();
  Grower<4, 5> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 4));
}

TEST(GrowerTest, G56) {
  Counters::initialize();
  Grower<5, 6> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(6, 5));
}

TEST(GrowerTest, G67) {
  Counters::initialize();
  Grower<6, 7> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(7, 6));
}

TEST(GrowerTest, G78) {
  Counters::initialize();
  Grower<7, 8> s;
  s.grow(false);
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(8, 7));
}
