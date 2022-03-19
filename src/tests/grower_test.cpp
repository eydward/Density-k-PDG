#include "../grower.h"

#include "../counters.h"
#include "../graph.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(GrowerTest, G23) {
  Graph::set_global_graph_info(2, 3);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
  }
}

TEST(GrowerTest, G24) {
  Graph::set_global_graph_info(2, 4);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(3, 2));
  }
}

TEST(GrowerTest, G25) {
  Graph::set_global_graph_info(2, 5);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
  }
}

TEST(GrowerTest, G26) {
  Graph::set_global_graph_info(2, 6);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 3));
  }
}

TEST(GrowerTest, G34) {
  Graph::set_global_graph_info(3, 4);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(4, 3));
  }
}

TEST(GrowerTest, G45) {
  Graph::set_global_graph_info(4, 5);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(5, 4));
  }
}

TEST(GrowerTest, G56) {
  Graph::set_global_graph_info(5, 6);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(6, 5));
  }
}

TEST(GrowerTest, G67) {
  Graph::set_global_graph_info(6, 7);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == Fraction(7, 6));
  }
}
