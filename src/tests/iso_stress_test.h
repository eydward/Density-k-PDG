// A stress test to verify the correctness of the isomorphism check algorithm.
// It's the most complex algorithm in this program so we want to use more than the unit tests
// and code review to confirm it's implemented correctly.
//
// The idea is to compare the result of Graph::is_isomorphic() and Graph::is_isomorphic_slow()
// and verify that they always match.
//
// Graph::is_isomorphic_slow() is a very simple bruteforce isomorphism check implementation,
// and we can easily verify its correctness by reviewing the code. So by using large number of
// graphs to verify Graph::is_isomorphic()==Graph::is_isomorphic_slow(), we gain high confidence
// that Graph::is_isomorphic() is implemented correctly.

#pragma once
#include "../graph.h"

class IsomorphismStressTest {
 private:
  const int k, n, c;
  std::mt19937 random_engine;
  void exit_assert(bool expected, bool actual, const Graph& g, const Graph& h, const char* msg);
  Graph get_one_graph(const uint8 edge_state[MAX_EDGES]);
  bool next_edge_state(uint8 edge_state[MAX_EDGES]);

  // The vertex indices in the corresponding edge candidate.
  // [0] is always NOT_IN_SET, [1] is always UNDIRECTED,
  // [2] to [k+1] are the vertices in the corresponding edge.
  uint8 edge_candidates_vidx[MAX_EDGES][MAX_VERTICES + 2];

  void verify_graphs(const Graph& g, const Graph& h);

 public:
  IsomorphismStressTest(int k_, int n_, int c_);

  void run();
};
