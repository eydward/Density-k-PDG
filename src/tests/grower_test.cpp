#include "../counters.h"
#include "../forbid_tk/graph_tk.h"
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

void verify_growth_result(int k, int n, Fraction min_theta, const std::string& min_theta_graph,
                          int num_threads) {
  Graph::set_global_graph_info(k, n);
  Counters::initialize();
  GrowerTk s1(num_threads, false, false, false, 0, 0);
  s1.grow();
  EXPECT_EQ(Counters::get_min_ratio(), min_theta);
  if (num_threads == 0) {
    // Check the min_theta_graph only when not using multi-threading, because different
    // threads could race and use a different graph to generate the same min_theta value.
    EXPECT_EQ(Counters::get_min_ratio_graph().serialize_edges(), min_theta_graph);
  }

  Counters::initialize();
  GrowerTk s2(num_threads, false, false, true, 0, 0);
  s2.grow();
  if (num_threads == 0) {
    EXPECT_TRUE(Counters::get_min_ratio() == min_theta);
  }

  Counters::initialize();
  GrowerTk s3(num_threads, false, true, false, 0, 0);
  s3.grow();
  if (num_threads == 0) {
    EXPECT_TRUE(Counters::get_min_ratio() == min_theta);
  }

  Counters::initialize();
  GrowerTk s4(num_threads, false, true, true, 0, 0);
  s4.grow();
  if (num_threads == 0) {
    EXPECT_TRUE(Counters::get_min_ratio() == min_theta);
  }

  // In addition to the above, also verify that the resulting growth graphs are identical
  // when different edge gen optimization combinations are used.
  if (num_threads == 0) {
    verify_array_equal(s1.get_results(), s2.get_results());
    verify_array_equal(s1.get_results(), s3.get_results());
    verify_array_equal(s1.get_results(), s4.get_results());
  }
}

void verify_growth_result_various_threads(int k, int n, Fraction min_theta,
                                          const std::string& min_theta_graph) {
  Graph::set_global_graph_info(k, n);
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    verify_growth_result(k, n, min_theta, min_theta_graph, num_threads);
  }
}

TEST(GrowerTest, Grow) {
  verify_growth_result_various_threads(2, 3, Fraction(3, 2), "{01>0, 02>0}");
  verify_growth_result_various_threads(2, 4, Fraction(3, 2), "{01>0, 02>0, 13>1, 23>2}");
  verify_growth_result_various_threads(2, 5, Fraction(5, 3),
                                       "{01>0, 02>0, 03>0, 14>1, 24>2, 34>3}");
  verify_growth_result_various_threads(2, 6, Fraction(5, 3),
                                       "{01>0, 02>0, 03>0, 14>1, 24>2, 34>3, 15>1, 25>2, 35>3}");
  verify_growth_result_various_threads(3, 4, Fraction(4, 3), "{012>0, 013>0, 023>0}");
  verify_growth_result_various_threads(4, 5, Fraction(5, 4), "{0123>0, 0124>0, 0134>0, 0234>0}");
  verify_growth_result_various_threads(5, 6, Fraction(6, 5),
                                       "{01234>0, 01235>0, 01245>0, 01345>0, 02345>0}");
  verify_growth_result_various_threads(
      6, 7, Fraction(7, 6), "{012345>0, 012346>0, 012356>0, 012456>0, 013456>0, 023456>0}");
  verify_growth_result_various_threads(
      7, 8, Fraction(8, 7),
      "{0123456>0, 0123457>0, 0123467>0, 0123567>0, 0124567>0, 0134567>0, 0234567>0}");
  verify_growth_result_various_threads(
      8, 9, Fraction(9, 8),
      "{01234567>0, 01234568>0, 01234578>0, 01234678>0, 01235678>0, 01245678>0, "
      "01345678>0, 02345678>0}");
  verify_growth_result(9, 10, Fraction(10, 9),
                       "{012345678>0, 012345679>0, 012345689>0, 012345789>0, 012346789>0, "
                       "012356789>0, 012456789>0, 013456789>0, 023456789>0}",
                       9);
}

// This test is slow to run so comment out by default.
TEST(GrowerTest, GrowSlow) {
  // verify_growth_result(10, 11, Fraction(11, 10),
  //                      "{0123456789>0, 012345678a>0, 012345679a>0, 012345689a>0, 012345789a>0, "
  //                      "012346789a>0, 012356789a>0, 012456789a>0, 013456789a>0, 023456789a>0}",
  //                      9);
  // verify_growth_result(
  //     11, 12, Fraction(12, 11),
  //     "{0123456789a>0, 0123456789b>0, 012345678ab>0, 012345679ab>0, 012345689ab>0, 012345789ab>0,
  //     " "012346789ab>0, 012356789ab>0, 012456789ab>0, 013456789ab>0, 023456789ab>0}", 9);
}

void verify_thetagraph_search(int k, int n, Fraction theta, int expected_thetagraph_count,
                              Fraction expected_min_theta) {
  Graph::set_global_graph_info(k, n);
  for (int num_threads = 0; num_threads < 10; num_threads++) {
    Counters::initialize();
    GrowerTk s1(num_threads, false, true, true, 0, 0, true, theta);
    s1.grow();
    EXPECT_EQ(Counters::get_min_ratio(), expected_min_theta);
    EXPECT_EQ(Counters::get_ratio_graph_count(), expected_thetagraph_count);
  }
}

TEST(GrowerTest, ThetaGraphSearch) {
  verify_thetagraph_search(2, 3, Fraction(3, 2), 4, Fraction(3, 2));
  verify_thetagraph_search(2, 3, Fraction(5, 4), 0, Fraction(5, 4));
  verify_thetagraph_search(2, 3, Fraction(2, 1), 10, Fraction(3, 2));
  verify_thetagraph_search(3, 4, Fraction(4, 3), 1, Fraction(4, 3));
  verify_thetagraph_search(3, 5, Fraction(5, 3), 3, Fraction(5, 3));
}

TEST(GrowerTest, WithLogging) {
  Graph::set_global_graph_info(2, 3);
  {
    std::stringstream log, log_detail, log_result;
    Counters::initialize(&log);
    GrowerTk s1(2, false, false, false, 0, 0, false, Fraction(1E8, 1));
    s1.set_logging(&log, &log_detail, &log_result);
    s1.set_stats_print_interval(1, 0);
    s1.grow();
    EXPECT_EQ(Counters::get_min_ratio(), Fraction(3, 2));
  }
  {
    std::stringstream log, log_detail, log_result;
    Counters::initialize(&log);
    GrowerTk s1(2, false, false, false, 0, 0, true, Fraction(3, 2));
    s1.set_logging(&log, &log_detail, &log_result);
    s1.set_stats_print_interval(1, 0);
    s1.grow();
    EXPECT_EQ(Counters::get_min_ratio(), Fraction(3, 2));
  }
  {
    std::stringstream log, log_detail, log_result;
    Counters::initialize(&log);
    GrowerTk s1(2, true, false, false, 0, 0, false, Fraction(1E8, 1));
    s1.set_logging(&log, &log_detail, &log_result);
    s1.set_stats_print_interval(1, 0);
    s1.grow();
    EXPECT_EQ(Counters::get_min_ratio(), Fraction(3, 1));
  }
}