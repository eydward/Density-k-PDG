#include "../edge_gen.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(EdgeGeneratorTest, Generate22) {
  Graph::set_global_graph_info(2, 2);
  EdgeCandidates ec(2);
  EXPECT_EQ(ec.edge_candidate_count, 1);
  EXPECT_EQ(ec.edge_candidates[0], 0b11);
  EXPECT_EQ(ec.edge_candidates_heads[0][0], NOT_IN_SET);
  EXPECT_EQ(ec.edge_candidates_heads[0][1], UNDIRECTED);
  EXPECT_EQ(ec.edge_candidates_heads[0][2], 0);
  EXPECT_EQ(ec.edge_candidates_heads[0][3], 1);

  EdgeGenerator edge_gen(ec, Graph());
  Graph copy;

  // 3 edges: {01} {01>0} {01>1}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{01}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{01>0}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{01>1}");

  // The enumeration has completed.
  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate23) {
  Graph::set_global_graph_info(2, 3);
  EdgeCandidates ec(3);
  EXPECT_EQ(ec.edge_candidate_count, 2);
  EXPECT_EQ(ec.edge_candidates[0], 0b101);
  EXPECT_EQ(ec.edge_candidates[1], 0b110);
  EXPECT_EQ(ec.edge_candidates_heads[0][0], NOT_IN_SET);
  EXPECT_EQ(ec.edge_candidates_heads[0][1], UNDIRECTED);
  EXPECT_EQ(ec.edge_candidates_heads[0][2], 0);
  EXPECT_EQ(ec.edge_candidates_heads[0][3], 2);
  EXPECT_EQ(ec.edge_candidates_heads[1][0], NOT_IN_SET);
  EXPECT_EQ(ec.edge_candidates_heads[1][1], UNDIRECTED);
  EXPECT_EQ(ec.edge_candidates_heads[1][2], 1);
  EXPECT_EQ(ec.edge_candidates_heads[1][3], 2);

  EdgeGenerator edge_gen(ec, Graph());
  Graph copy;

  // First 3: {02} {02>0} {02>2}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{02}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{02>0}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{02>2}");

  // Next 12: {12} {02, 12} {02>0, 12} {02>2, 12}
  //          {12>1} {02, 12>1} {02>0, 12>1} {02>2, 12>1}
  //          {12>2} {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  for (int i1 = 0; i1 <= 2; i1++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i1 == 0 ? UNDIRECTED : i1));

    for (int i0 = -1; i0 <= 1; i0++) {
      EXPECT_TRUE(edge_gen.next(copy));
      EXPECT_EQ(copy.edge_count, 2);
      EXPECT_EQ(copy.edges[0].vertex_set, 0b101);
      EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i0 == 1 ? 2 : i0));
      EXPECT_EQ(copy.edges[1].vertex_set, 0b110);
      EXPECT_EQ(copy.edges[1].head_vertex, static_cast<uint8>(i1 == 0 ? UNDIRECTED : i1));
    }
  }

  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate23WithSkip) {
  Graph::set_global_graph_info(2, 3);
  EdgeCandidates ec(3);
  EdgeGenerator edge_gen(ec, Graph());
  Graph copy;

  // First 3: {02} {02>0} {02>2}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{02}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{02>0}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{02>2}");

  // Next: {12}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.edge_count, 1);
  EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
  EXPECT_EQ(copy.edges[0].head_vertex, UNDIRECTED);

  // Notify to skip, so the next 3 are skipped: {02, 12} {02>0, 12} {02>2, 12}.
  edge_gen.notify_contain_tk_skip();

  // Next: {12>1}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.edge_count, 1);
  EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
  EXPECT_EQ(copy.edges[0].head_vertex, 1);

  // Notify to skip, so the next 3 are skipped: {02, 12>1} {02>0, 12>1} {02>2, 12>1}.
  edge_gen.notify_contain_tk_skip();

  // Next 4: {12>2} {02, 12>2} {02>0, 12>2} {02>2, 12>2}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.edge_count, 1);
  EXPECT_EQ(copy.edges[0].vertex_set, 0b110);
  EXPECT_EQ(copy.edges[0].head_vertex, 2);

  for (int i0 = -1; i0 <= 1; i0++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 2);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b101);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i0 == 1 ? 2 : i0));
    EXPECT_EQ(copy.edges[1].vertex_set, 0b110);
    EXPECT_EQ(copy.edges[1].head_vertex, 2);
  }

  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate33) {
  Graph::set_global_graph_info(3, 3);
  EdgeCandidates ec(3);
  EXPECT_EQ(ec.edge_candidate_count, 1);
  EXPECT_EQ(ec.edge_candidates[0], 0b0111);
  EXPECT_EQ(ec.edge_candidates_heads[0][0], NOT_IN_SET);
  EXPECT_EQ(ec.edge_candidates_heads[0][1], UNDIRECTED);
  EXPECT_EQ(ec.edge_candidates_heads[0][2], 0);
  EXPECT_EQ(ec.edge_candidates_heads[0][3], 1);
  EXPECT_EQ(ec.edge_candidates_heads[0][4], 2);

  EdgeGenerator edge_gen(ec, Graph());
  Graph copy;

  // 4 edges: {012} {012>0} {012>1} {012>2}
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{012}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{012>0}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{012>1}");
  EXPECT_TRUE(edge_gen.next(copy));
  EXPECT_EQ(copy.serialize_edges(), "{012>2}");

  EXPECT_FALSE(edge_gen.next(copy));
}

TEST(EdgeGeneratorTest, Generate35) {
  Graph::set_global_graph_info(3, 5);
  EdgeCandidates ec(5);
  EXPECT_EQ(ec.edge_candidate_count, 6);
  EXPECT_EQ(ec.edge_candidates[0], 0b0010011);
  EXPECT_EQ(ec.edge_candidates[1], 0b0010101);
  EXPECT_EQ(ec.edge_candidates[2], 0b0010110);
  EXPECT_EQ(ec.edge_candidates[3], 0b0011001);
  EXPECT_EQ(ec.edge_candidates[4], 0b0011010);
  EXPECT_EQ(ec.edge_candidates[5], 0b0011100);
  for (int v = 0; v < 6; v++) {
    EXPECT_EQ(ec.edge_candidates_heads[v][0], NOT_IN_SET);
    EXPECT_EQ(ec.edge_candidates_heads[v][1], UNDIRECTED);
  }
  EXPECT_EQ(ec.edge_candidates_heads[0][2], 0);
  EXPECT_EQ(ec.edge_candidates_heads[0][3], 1);
  EXPECT_EQ(ec.edge_candidates_heads[0][4], 4);
  EXPECT_EQ(ec.edge_candidates_heads[1][2], 0);
  EXPECT_EQ(ec.edge_candidates_heads[1][3], 2);
  EXPECT_EQ(ec.edge_candidates_heads[1][4], 4);
  EXPECT_EQ(ec.edge_candidates_heads[2][2], 1);
  EXPECT_EQ(ec.edge_candidates_heads[2][3], 2);
  EXPECT_EQ(ec.edge_candidates_heads[2][4], 4);
  EXPECT_EQ(ec.edge_candidates_heads[3][2], 0);
  EXPECT_EQ(ec.edge_candidates_heads[3][3], 3);
  EXPECT_EQ(ec.edge_candidates_heads[3][4], 4);
  EXPECT_EQ(ec.edge_candidates_heads[4][2], 1);
  EXPECT_EQ(ec.edge_candidates_heads[4][3], 3);
  EXPECT_EQ(ec.edge_candidates_heads[4][4], 4);
  EXPECT_EQ(ec.edge_candidates_heads[5][2], 2);
  EXPECT_EQ(ec.edge_candidates_heads[5][3], 3);
  EXPECT_EQ(ec.edge_candidates_heads[5][4], 4);

  EdgeGenerator edge_gen(ec, Graph());
  edge_gen.print_debug(std::cout, true, 0);
  Graph copy;

  int count = 0;
  while (edge_gen.next(copy)) {
    ++count;
  }

  int expected = 5 * 5 * 5 * 5 * 5 * 5 - 1;
  EXPECT_EQ(count, expected);
}

TEST(EdgeGeneratorTest, Generate27) {
  Graph::set_global_graph_info(2, 7);
  EdgeCandidates ec(7);
  EXPECT_EQ(ec.edge_candidate_count, 6);
  EXPECT_EQ(ec.edge_candidates[0], 0b01000001);
  EXPECT_EQ(ec.edge_candidates[1], 0b01000010);
  EXPECT_EQ(ec.edge_candidates[2], 0b01000100);
  EXPECT_EQ(ec.edge_candidates[3], 0b01001000);
  EXPECT_EQ(ec.edge_candidates[4], 0b01010000);
  EXPECT_EQ(ec.edge_candidates[5], 0b01100000);
  for (int v = 0; v < 6; v++) {
    EXPECT_EQ(ec.edge_candidates_heads[v][0], NOT_IN_SET);
    EXPECT_EQ(ec.edge_candidates_heads[v][1], UNDIRECTED);
    EXPECT_EQ(ec.edge_candidates_heads[v][2], v);
    EXPECT_EQ(ec.edge_candidates_heads[v][3], 6);
  }

  EdgeGenerator edge_gen(ec, Graph());
  Graph copy;

  int count = 0;
  while (edge_gen.next(copy)) {
    ++count;
  }

  int expected = 4 * 4 * 4 * 4 * 4 * 4 - 1;
  EXPECT_EQ(count, expected);
}

TEST(EdgeGeneratorTest, Generate45) {
  Graph::set_global_graph_info(4, 5);
  EdgeCandidates ec(5);
  EdgeGenerator edge_gen(ec, Graph());
  Graph copy;

  // First 5: {0124} {0124>0} {0124>1} {0124>2} {0124>4}
  for (int i = -1; i <= 3; i++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b10111);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i == 3 ? 4 : i));
  }

  // Next 30:
  //   {0134} {0124, 0134} {0124>0, 0134} {0124>1, 0134} {0124>2, 0134} {0124>4, 0134}
  //   {0134>0} {0124, 0134>0} {0124>0, 0134>0} {0124>1, 0134>0} {0124>2, 0134>0} {0124>4, 0134>0}
  //   ...
  //   {0134>4} {0124, 0134>4} {0124>0, 0134>4} {0124>1, 0134>4} {0124>2, 0134>4} {0124>4, 0134>4}
  for (int i1 = -1; i1 <= 3; i1++) {
    EXPECT_TRUE(edge_gen.next(copy));
    EXPECT_EQ(copy.edge_count, 1);
    EXPECT_EQ(copy.edges[0].vertex_set, 0b11011);
    EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));

    for (int i0 = -1; i0 <= 3; i0++) {
      EXPECT_TRUE(edge_gen.next(copy));
      EXPECT_EQ(copy.edge_count, 2);
      EXPECT_EQ(copy.edges[0].vertex_set, 0b10111);
      EXPECT_EQ(copy.edges[0].head_vertex, static_cast<uint8>(i0 == 3 ? 4 : i0));
      EXPECT_EQ(copy.edges[1].vertex_set, 0b11011);
      EXPECT_EQ(copy.edges[1].head_vertex, static_cast<uint8>(i1 >= 2 ? i1 + 1 : i1));
    }
  }
}

TEST(EdgeGeneratorTest, NotifyContainsT2) {
  Graph::set_global_graph_info(2, 6);
  EdgeCandidates ec(6);
  Graph base;
  EXPECT_TRUE(Graph::parse_edges("{23>2}", base));
  EdgeGenerator edge_gen(ec, base);

  Graph copy;
  // Skip generated graphs that don't contain T_2
  for (int i = 1; i < 4 * 4 * 4 + 4 * 4; i++) {
    EXPECT_TRUE(edge_gen.next(copy, false));
    EXPECT_FALSE(copy.contains_Tk(5));
  }

  // The next one contains T_2
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_TRUE(copy.contains_Tk(5));
  EXPECT_EQ(copy.serialize_edges(), "{23>2, 25, 35}");

  // Notify contains T_2, it should skip over a bunch of graphs.
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{23>2, 25>2, 35}");
  EXPECT_TRUE(copy.contains_Tk(5));

  // Again.
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{23>2, 25>5, 35}");
  EXPECT_TRUE(copy.contains_Tk(5));

  // Again.
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{23>2, 35>3}");
  EXPECT_FALSE(copy.contains_Tk(5));
}

TEST(EdgeGeneratorTest, NotifyContainsTk3) {
  Graph::set_global_graph_info(3, 5);
  EdgeCandidates ec(5);
  Graph base;
  EXPECT_TRUE(Graph::parse_edges("{123}", base));
  EdgeGenerator edge_gen(ec, base);

  Graph copy;
  // Skip generated graphs that don't contain T_3
  for (int i = 1; i < 32; i++) {
    EXPECT_TRUE(edge_gen.next(copy, false));
    EXPECT_FALSE(copy.contains_Tk(4));
  }

  // Next one contains T_3
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_TRUE(copy.contains_Tk(4));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014>0, 024, 124}");

  // Notify contains T_3, should be no-op
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014>1, 024, 124}");
  EXPECT_TRUE(copy.contains_Tk(4));
  // Notify contains T_3, should be no-op again
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014>4, 024, 124}");
  EXPECT_FALSE(copy.contains_Tk(4));

  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 024>0, 124}");
  EXPECT_FALSE(copy.contains_Tk(4));

  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014, 024>0, 124}");
  EXPECT_TRUE(copy.contains_Tk(4));

  // Notify contains T_3, should be no-op again
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014>0, 024>0, 124}");
  EXPECT_TRUE(copy.contains_Tk(4));
  // Notify contains T_3, should be no-op again
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014>1, 024>0, 124}");
  EXPECT_TRUE(copy.contains_Tk(4));
  // Notify contains T_3, should be no-op again
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 014>4, 024>0, 124}");
  EXPECT_TRUE(copy.contains_Tk(4));
  // Notify contains T_3, should be no-op again
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 024>2, 124}");
  EXPECT_FALSE(copy.contains_Tk(4));

  // Skip a lot of graphs.
  for (int i = 1; i < 5 * 5 * 5 * 5 * 5 + 5 * 5 * 5; i++) {
    EXPECT_TRUE(edge_gen.next(copy, false));
  }

  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 024>2, 124, 034, 234}");
  EXPECT_TRUE(copy.contains_Tk(4));
  // Notify contains T_3, it should skip over a bunch of graphs.
  edge_gen.notify_contain_tk_skip();
  EXPECT_TRUE(edge_gen.next(copy, false));
  EXPECT_EQ(copy.serialize_edges(), "{123, 024>4, 124, 034, 234}");
  EXPECT_FALSE(copy.contains_Tk(5));
}

TEST(EdgeGeneratorTest, MinTheta1) {
  for (int k = 2; k <= 4; k++) {
    std::string edge;
    switch (k) {
      case 2:
        edge = "{01}";
        break;
      case 3:
        edge = "{012}";
        break;
      case 4:
        edge = "{0123}";
        break;
    }
    for (int n = k + 1; n <= 7; n++) {
      Graph::set_global_graph_info(k, n);
      Graph base;
      EXPECT_TRUE(Graph::parse_edges(edge, base));
      EdgeCandidates ec(n);
      EdgeGenerator edge_gen(ec, base);

      Graph copy;
      // Given the known theta is already 1, no graph can possibly do better than that,
      // so we expect edge gen just optimize everything away and declare enumeration done.
      EXPECT_FALSE(edge_gen.next(copy, true, Fraction(1, 1)));
    }
  }
}

TEST(EdgeGeneratorTest, MinTheta2) {
  Graph::set_global_graph_info(2, 3);
  Graph base;
  EXPECT_TRUE(Graph::parse_edges("{01}", base));
  EdgeCandidates ec(3);
  EdgeGenerator edge_gen(ec, base);

  Graph copy;
  // Given that the know min_theta is 2, if we add one edge to the new graph, it won't produce
  // a smaller theta, so it should have two new edges. In addition, if all edges in the new
  // graph are undirected, the theta won't be smaller, therefore the first graph it generates
  // should have two edges with one directed.
  EXPECT_TRUE(edge_gen.next(copy, true, Fraction(2, 1)));
  EXPECT_EQ(copy.serialize_edges(), "{01, 02>0, 12}");

  // Generate the next one, doesn't skip anything.
  EXPECT_TRUE(edge_gen.next(copy, true, Fraction(2, 1)));
  EXPECT_EQ(copy.serialize_edges(), "{01, 02>2, 12}");
  // Generate the next one, doesn't skip anything.
  EXPECT_TRUE(edge_gen.next(copy, true, Fraction(2, 1)));
  EXPECT_EQ(copy.serialize_edges(), "{01, 02, 12>1}");
  // Generate the next one, doesn't skip anything.
  EXPECT_TRUE(edge_gen.next(copy, true, Fraction(2, 1)));
  EXPECT_EQ(copy.serialize_edges(), "{01, 02>0, 12>1}");

  // Run to the end.
  for (int i = 0; i < 4; i++) {
    EXPECT_TRUE(edge_gen.next(copy, true, Fraction(2, 1)));
  }
  EXPECT_FALSE(edge_gen.next(copy, true, Fraction(2, 1)));
}

TEST(EdgeGeneratorTest, MinTheta3) {
  Graph::set_global_graph_info(3, 4);
  Graph base;
  EXPECT_TRUE(Graph::parse_edges("{012>2}", base));
  EdgeCandidates ec(4);
  EdgeGenerator edge_gen(ec, base);

  Graph copy;
  // Given that the know min_theta is 3/2, if we add one edge to the new graph, it won't produce
  // a smaller theta, so it should have two new edges. In addition, if either new edge in the new
  // graph is undirected, the theta won't be smaller, therefore the first graph it generates
  // should have 3 edges, all directed.
  EXPECT_TRUE(edge_gen.next(copy, true, Fraction(3, 2)));
  EXPECT_EQ(copy.serialize_edges(), "{012>2, 013>0, 023>0}");

  // Next set, different head direction of the above.
  for (int i = 0; i < 8; i++) {
    EXPECT_TRUE(edge_gen.next(copy, true, Fraction(3, 2)));
  }
  EXPECT_EQ(copy.serialize_edges(), "{012>2, 013>3, 023>3}");

  // Next should be "{012>2, 123}". But it doesn't produce smaller theta. So it should skip some,
  // to get two directed edges.
  EXPECT_TRUE(edge_gen.next(copy, true, Fraction(3, 2)));
  EXPECT_EQ(copy.serialize_edges(), "{012>2, 013>0, 023>0, 123}");
}