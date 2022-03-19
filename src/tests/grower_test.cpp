#include "../grower.h"

#include "../counters.h"
#include "../graph.h"
#include "gtest/gtest.h"

using namespace testing;

void verify_growth_result(int k, int n, Fraction min_theta) {
  Graph::set_global_graph_info(k, n);
  Counters::initialize();
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Grower s(num_threads, 0, 0);
    s.grow();
    EXPECT_TRUE(Counters::get_min_theta() == min_theta);
  }
}

TEST(GrowerTest, Grow) {
  verify_growth_result(2, 3, Fraction(3, 2));
  verify_growth_result(2, 4, Fraction(3, 2));
  verify_growth_result(2, 5, Fraction(5, 3));
  verify_growth_result(2, 6, Fraction(5, 3));
  // verify_growth_result(2, 7, Fraction(7, 4));
  verify_growth_result(3, 4, Fraction(4, 3));
  verify_growth_result(4, 5, Fraction(5, 4));
  verify_growth_result(5, 6, Fraction(6, 5));
  verify_growth_result(6, 7, Fraction(7, 6));
}
