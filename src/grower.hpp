#pragma once
#include "counters.h"
#include "grower.h"

// Find all canonical isomorphism class representations with up to max_n vertices.
template <int K, int N>
void Grower<K, N>::grow() {
  static_assert(N <= 8);
  print_config(std::cout);
  if (log != nullptr) {
    print_config(*log);
  }

  // Initialize empty graph with k-1 vertices.
  G* g = allocator.get_current_graph_from_allocator();
  g->init();
  canonicals[K - 1].insert(g);
  Counters::observe_theta(*g);
  allocator.mark_current_graph_used();

  for (int n = K; n <= N; n++) {
    grow_step(n);
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

  for (const G* g : canonicals[n - 1]) {
    edge_gen.reset_enumeration();
    assert(n == K || (g->is_canonical && g->vertex_count == n - 1));
    assert(g->vertices[n - 1].get_degrees() == 0);

    G* copy = allocator.get_current_graph_from_allocator();
    // Loop through all ((K+1)^\binom{n-1}{k-1} - 1) edge combinations, add them to g, and check
    // add to canonicals unless it's isomorphic to an existing one.
    while (edge_gen.next()) {
      g->copy_without_init(copy);
      for (int i = 0; i < edge_gen.edge_count; i++) {
        copy->add_edge(edge_gen.edges[i]);
      }
      if (copy->contains_Tk(n - 1)) {
        edge_gen.notify_contain_tk_skip();
        continue;
      }

      copy->init();
      copy->canonicalize();

      if (!canonicals[n].contains(copy)) {
        canonicals[n].insert(copy);
        Counters::observe_theta(*copy);
        allocator.mark_current_graph_used();
        copy = allocator.get_current_graph_from_allocator();
      }
    }
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
      for (const G* g : canonicals[i]) {
        g->print_concise(os);
      }
    }
  }
  os << "Total canonicals= " << total_canonicals << "\n";
}
