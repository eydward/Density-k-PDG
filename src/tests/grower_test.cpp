#include "../grower.h"

#include "../counters.h"
#include "../graph.h"
#include "gtest/gtest.h"

using namespace testing;

void verify_array_equal(const std::vector<std::tuple<int, Graph, Graph>>& a,
                        const std::vector<std::tuple<int, Graph, Graph>>& b) {
  ASSERT_EQ(a.size(), b.size());
  for (size_t i = 0; i < a.size(); i++) {
    ASSERT_EQ(std::get<0>(a[i]), std::get<0>(b[i]));
    ASSERT_EQ(std::get<1>(a[i]).serialize_edges(), std::get<1>(b[i]).serialize_edges());
    ASSERT_EQ(std::get<2>(a[i]).serialize_edges(), std::get<2>(b[i]).serialize_edges());
  }
}

void verify_growth_result(int k, int n, Fraction min_theta,
                          const std::string& min_theta_graph = "") {
  Graph::set_global_graph_info(k, n);
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Counters::initialize();
    Grower s1(num_threads, false, false, false, 0, 0);
    s1.grow();
    EXPECT_EQ(Counters::get_min_theta(), min_theta);
    if (num_threads == 0) {
      // Check the min_theta_graph only when not using multi-threading, because different
      // threads could race and use a different graph to generate the same min_theta value.
      EXPECT_EQ(Counters::get_min_theta_graph().serialize_edges(), min_theta_graph);
    }

    Counters::initialize();
    Grower s2(num_threads, false, false, true, 0, 0);
    s2.grow();
    if (num_threads == 0) {
      EXPECT_TRUE(Counters::get_min_theta() == min_theta);
    }

    Counters::initialize();
    Grower s3(num_threads, false, true, false, 0, 0);
    s3.grow();
    if (num_threads == 0) {
      EXPECT_TRUE(Counters::get_min_theta() == min_theta);
    }

    Counters::initialize();
    Grower s4(num_threads, false, true, true, 0, 0);
    s4.grow();
    if (num_threads == 0) {
      EXPECT_TRUE(Counters::get_min_theta() == min_theta);
    }

    // In addition to the above, also verify that the resulting growth graphs are identical
    // when different edge gen optimization combinations are used.
    if (num_threads == 0) {
      verify_array_equal(s1.get_results(), s2.get_results());
      verify_array_equal(s1.get_results(), s3.get_results());
      verify_array_equal(s1.get_results(), s4.get_results());
    }
  }
}

TEST(GrowerTest, Grow) {
  verify_growth_result(2, 3, Fraction(3, 2), "{01>0, 02>0}");
  verify_growth_result(2, 4, Fraction(3, 2), "{01>0, 02>0, 13>1, 23>2}");
  verify_growth_result(2, 5, Fraction(5, 3), "{01>0, 02>0, 03>0, 14>1, 24>2, 34>3}");
  verify_growth_result(2, 6, Fraction(5, 3),
                       "{01>0, 02>0, 03>0, 14>1, 24>2, 34>3, 15>1, 25>2, 35>3}");
  verify_growth_result(3, 4, Fraction(4, 3), "{012>0, 013>0, 023>0}");
  verify_growth_result(4, 5, Fraction(5, 4), "{0123>0, 0124>0, 0134>0, 0234>0}");
  verify_growth_result(5, 6, Fraction(6, 5), "{01234>0, 01235>0, 01245>0, 01345>0, 02345>0}");
  verify_growth_result(6, 7, Fraction(7, 6),
                       "{012345>0, 012346>0, 012356>0, 012456>0, 013456>0, 023456>0}");
}

void verify_thetagraph_search(int k, int n, Fraction theta, int expected_thetagraph_count,
                              Fraction expected_min_theta) {
  Graph::set_global_graph_info(k, n);
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Counters::initialize();
    Grower s1(num_threads, false, true, true, 0, 0, true, theta);
    s1.grow();
    std::cout << "*** " << Counters::get_min_theta().to_string() << "\n";
    EXPECT_EQ(Counters::get_min_theta(), expected_min_theta);
    EXPECT_EQ(Counters::get_thetagraph_count(), expected_thetagraph_count);
  }
}

TEST(GrowerTest, ThetaGraphSearch) {
  verify_thetagraph_search(2, 3, Fraction(3, 2), 4, Fraction(3, 2));
  verify_thetagraph_search(2, 3, Fraction(5, 4), 0, Fraction(5, 4));
  verify_thetagraph_search(2, 3, Fraction(2, 1), 10, Fraction(3, 2));
  verify_thetagraph_search(3, 4, Fraction(4, 3), 1, Fraction(4, 3));
  verify_thetagraph_search(3, 5, Fraction(5, 3), 3, Fraction(5, 3));
}