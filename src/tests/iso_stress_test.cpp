#include "iso_stress_test.h"

// If the number of graphs is below this threshold, we conduct a full test by going
// through all possible graphs for the (k,n) combination. Otherwise we select a random subset
// of all possible graphs.
constexpr int FULL_TEST_THRESHOLD = 1000;
constexpr uint8 NOT_IN_SET = 0xEE;

IsomorphismStressTest::IsomorphismStressTest(int k_, int n_, int c_) : k(k_), n(n_), c(c_) {
  Graph::set_global_graph_info(k, n);
  assert(Graph::TOTAL_EDGES == Graph::VERTEX_MASKS[k].mask_count);
  for (int e = 0; e < Graph::TOTAL_EDGES; e++) {
    int vidx = 0;
    edge_candidates_vidx[e][vidx++] = NOT_IN_SET;
    edge_candidates_vidx[e][vidx++] = UNDIRECTED;
    for (uint8 i = 0; i <= n - 1; i++) {
      if ((Graph::VERTEX_MASKS[k].masks[e] & (1 << i)) != 0) {
        edge_candidates_vidx[e][vidx++] = i;
      }
    }
    assert(vidx == k + 2);
  }
}

void IsomorphismStressTest::exit_assert(bool expected, bool actual, const Graph& g, const Graph& h,
                                        const char* msg) {
  if (expected != actual) {
    std::cout << "Expected=" << expected << ", actual=" << actual << ": " << msg << "\n";
    g.print();
    h.print();
    exit(-1);
  }
}

Graph IsomorphismStressTest::get_one_graph(const uint8 edge_state[MAX_EDGES]) {
  Graph g;
  for (int i = 0; i < Graph::TOTAL_EDGES; i++) {
    if (edge_state[i] != 0) {
      Edge e;
      e.vertex_set = Graph::VERTEX_MASKS[k].masks[i];
      e.head_vertex = edge_candidates_vidx[i][edge_state[i]];
      g.add_edge(e);
    }
  }
  // g.print_concise(std::cout);
  exit_assert(true, g.is_isomorphic_slow(g), g, g, "slow!=self");
  GraphInvariants gi;
  g.canonicalize(gi, c);
  exit_assert(true, g.is_isomorphic_slow(g), g, g, "slow!=self after canonicalize");
  exit_assert(true, g.is_isomorphic(g), g, g, "iso!=self");
  return g;
}

bool IsomorphismStressTest::next_edge_state(uint8 edge_state[MAX_EDGES]) {
  for (int i = 0; i < Graph::TOTAL_EDGES; i++) {
    ++edge_state[i];
    if (edge_state[i] != k + 2) {
      return true;
    }
    edge_state[i] = 0;
  }
  return false;
}

void IsomorphismStressTest::run() {
  // Total number of graphs. For each possible edge, its state can be directed with
  // k head choices, or undirected, or not in graph. Thus (k+2)^e.
  double total_graphs = pow(k + 2, Graph::TOTAL_EDGES);
  bool use_sampling = total_graphs > FULL_TEST_THRESHOLD;
  // If we need to sample, the number of graphs sampled should be close to FULL_TEST_THRESHOLD.
  double sampling_ratio = FULL_TEST_THRESHOLD / total_graphs;

  // These are used to generate random numbers for sampling purpose.
  std::uniform_real_distribution<double> uniform(0.0, 1.0);
  std::default_random_engine re;

  std::cout << "TEST k=" << k << ", n=" << n << ", c=" << c << ", e=" << Graph::TOTAL_EDGES
            << ", graph=" << total_graphs << ", use_sampling=" << use_sampling
            << ", sampling ratio=" << (sampling_ratio > 1.0 ? 1.0 : sampling_ratio)
            << ": Creating graphs\n";

  std::vector<Graph> graphs;
  uint8 edge_state[MAX_EDGES]{0};
  if (!use_sampling) {
    do {
      graphs.push_back(get_one_graph(edge_state));
    } while (next_edge_state(edge_state));

    for (int i = 0; i < graphs.size(); i++) {
      for (int j = i + 1; j < graphs.size(); j++) {
        const Graph& g = graphs[i];
        const Graph& h = graphs[j];
        bool iso_gh = g.is_isomorphic(h);
        bool iso_gh_slow = g.is_isomorphic_slow(h);
        bool iso_hg = h.is_isomorphic(g);
        bool iso_hg_slow = h.is_isomorphic_slow(g);
        exit_assert(iso_gh, iso_gh_slow, g, h, "iso!=slow");
        exit_assert(iso_hg, iso_hg_slow, h, g, "iso!=slow");
        exit_assert(iso_gh_slow, iso_hg_slow, g, h, "slow not commutative");
      }
    }
  }
}