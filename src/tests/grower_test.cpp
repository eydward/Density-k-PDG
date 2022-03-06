#include "../grower.hpp"

#include "../graph.hpp"
#include "gtest/gtest.h"

using namespace testing;

TEST(GrowerTest, G27) {
  Graph::set_global_graph_info(2, 3);
  Grower s;
  s.grow();
  EXPECT_EQ(s.canonicals[0].size(), 0);
  EXPECT_EQ(s.canonicals[1].size(), 1);
  EXPECT_EQ(s.canonicals[2].size(), 2);
  EXPECT_EQ(s.canonicals[3].size(), 7);
}

TEST(GrowerTest, G23) {
  Graph::set_global_graph_info(2, 3);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}

TEST(GrowerTest, G24) {
  Graph::set_global_graph_info(2, 4);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
}

TEST(GrowerTest, G25) {
  Graph::set_global_graph_info(2, 5);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
}

TEST(GrowerTest, G26) {
  Graph::set_global_graph_info(2, 6);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
}

TEST(GrowerTest, G34) {
  Graph::set_global_graph_info(3, 4);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(4, 3));
}

TEST(GrowerTest, G45) {
  Graph::set_global_graph_info(4, 5);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 4));
}

TEST(GrowerTest, G56) {
  Graph::set_global_graph_info(5, 6);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(6, 5));
}

TEST(GrowerTest, G67) {
  Graph::set_global_graph_info(6, 7);
  Counters::initialize();
  Grower s;
  s.grow();
  EXPECT_TRUE(Counters::get_min_theta() == Fraction(7, 6));
}
