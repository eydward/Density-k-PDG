#pragma once
#include "counters.h"
#include "grower.h"

// Find all canonical isomorphism class representations with up to max_n vertices.
template <int K, int N>
void Grower<K, N>::grow(bool alt_growth_impl) {
  static_assert(N <= 8);
  print_config(std::cout);
  if (log != nullptr) {
    print_config(*log);
  }

  // Initialize empty graph with k-1 vertices.
  G g;
  g.init();
  canonicals[K - 1].insert(g);
  Counters::observe_theta(g);

  for (int n = K; n <= N; n++) {
    if (alt_growth_impl) {
      grow_step_impl_alt(n);
    } else {
      grow_step(n);
    }
  }
}

// Debug print the content of the canonicals after the growth.
// If print_graphs==true, print stats and all graphs. Otherwise prints stats only.
template <int K, int N>
void Grower<K, N>::print(bool print_graphs) {
  print_state_to_stream(print_graphs, std::cout);
  if (log != nullptr) {
    print_state_to_stream(print_graphs, *log);
  }
}

// Constructs all non-isomorphic graphs with n vertices that are T_k-free,
// and add them to the canonicals. Before calling this, all such graphs
// with <n vertices must already be in the canonicals.
// Note all edges added in this step contains vertex (n-1).
template <int K, int N>
void Grower<K, N>::grow_step(int n) {
  edge_gen.initialize(K, n);

  for (const G& g : canonicals[n - 1]) {
    edge_gen.reset_enumeration();
    assert(n == K || (g.is_canonical && g.vertex_count == n - 1));
    assert(g.vertices[n - 1].get_degrees() == 0);

    G copy;
    // Loop through all ((K+1)^\binom{n-1}{k-1} - 1) edge combinations, add them to g, and check
    // add to canonicals unless it's isomorphic to an existing one.
    while (edge_gen.next()) {
      g.copy_without_init(copy);
      for (int i = 0; i < edge_gen.edge_count; i++) {
        copy.add_edge(edge_gen.edges[i]);
      }
      if (copy.contains_Tk(n - 1)) continue;

      copy.init();
      copy.canonicalize();

      if (!canonicals[n].contains(copy)) {
        canonicals[n].insert(copy);
        Counters::observe_theta(copy);
      }
    }
  }
}

// Constructs all non-isomorphic graphs with n vertices that are T_k-free,
// and add them to the canonicals. Before calling this, all such graphs
// with <n vertices must already be in the canonicals.
// Note all edges added in this step contains vertex (n-1).
//
// This is an alternative implementation of grow_step() that uses a different growth strategy.
template <int K, int N>
void Grower<K, N>::grow_step_impl_alt(int n) {
  // First find all edge candidates through the new vertex.
  std::vector<uint8> edge_candidates;
  for (uint8 mask = 0; mask < (1 << (n - 1)); mask++) {
    if (__builtin_popcount(mask) == K - 1) {
      edge_candidates.push_back(mask | (1 << (n - 1)));
    }
  }
  assert(edge_candidates.size() == compute_binom(n - 1, K - 1));

  for (const G& g : canonicals[n - 1]) {
    assert(n == K || (g.is_canonical && g.vertex_count == n - 1));
    assert(g.vertices[n - 1].get_degrees() == 0);

    // Add one edge at a time:
    // (1) current = {g}, next = empty.
    // (2) for each graph in current, for each edge candidate, try to add. If can, put in next.
    // (3) Add everything in current to canonicals, and move next to current.
    // Repeat for m times.
    std::unordered_set<G, GraphHasher, GraphComparer> current;
    current.insert(g);
    std::unordered_set<G, GraphHasher, GraphComparer> next;

    G copy;
    for (int i = 0; i < edge_candidates.size(); i++) {
      for (const G& start : current) {
        for (const uint8 edge : edge_candidates) {
          if (start.edge_allowed(edge)) {
            for (int head = -1; head < n; head++) {
              if (head >= 0 && (edge & (1 << head)) == 0) continue;
              start.copy_without_init(copy);
              copy.add_edge(Edge(edge, head < 0 ? UNDIRECTED : head));
              if (copy.contains_Tk(n - 1)) continue;

              copy.init();
              copy.canonicalize();

              if (!canonicals[n].contains(copy) && !current.contains(copy) &&
                  !next.contains(copy)) {
                next.insert(copy);
                Counters::observe_theta(copy);
              }
            }
          }
        }
      }

      // Put content of current into canonical, put content of next into current.
      if (i > 0) {
        canonicals[n].merge(current);
      } else {
        current.clear();
      }
      assert(current.empty());
      current.merge(next);
      assert(next.empty());
    }
    canonicals[n].merge(current);
  }
}

template <int K, int N>
void Grower<K, N>::print_config(std::ostream& os) {
  os << "Searching for all T_k-free k-PDGs\n    K= " << K
     << " (number of vertices in each edge)\n    N= " << N
     << " (total number of vertices in each graph)\n    E= " << G::MAX_EDGES
     << " (maximum possible number of edges in each graph)\n";
}
template <int K, int N>
void Grower<K, N>::print_state_to_stream(bool print_graphs, std::ostream& os) {
  uint64 total_canonicals = 0;
  for (int i = 0; i < N + 1; i++) {
    os << "order=" << i << " : canonicals= " << canonicals[i].size() << "\n";
    total_canonicals += canonicals[i].size();
    if (print_graphs) {
      for (const G& g : canonicals[i]) {
        g.print_concise(os);
      }
    }
  }
  os << "Total canonicals= " << total_canonicals << "\n";
}
