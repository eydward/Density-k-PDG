#include "grower.h"

#include "counters.h"

// Find all canonical isomorphism class representations with up to max_n vertices.
void Grower::grow() {
  assert(Graph::N <= 7);
  print_config(std::cout);
  if (log != nullptr) {
    print_config(*log);
  }

  // Initialize empty graph with k-1 vertices.
  Graph* g = allocator.get_current_graph_from_allocator();
  VertexSignature vs[MAX_VERTICES];
  g->compute_vertex_signature(vs);
  g->compute_graph_hash(vs);
  canonicals[Graph::K - 1].insert(g);
  Counters::observe_theta(*g);
  allocator.mark_current_graph_used();

  for (int n = Graph::K; n <= Graph::N; n++) {
    grow_step(n);
  }
}

// Debug print the content of the canonicals after the growth.
// If print_graphs==true, print stats and all graphs. Otherwise prints stats only.
void Grower::print(bool print_graphs) {
  print_state_to_stream(print_graphs, std::cout);
  if (log != nullptr) {
    print_state_to_stream(print_graphs, *log);
  }
}

// Constructs all non-isomorphic graphs with n vertices that are T_k-free,
// and add them to the canonicals. Before calling this, all such graphs
// with <n vertices must already be in the canonicals.
// Note all edges added in this step contains vertex (n-1).
void Grower::grow_step(int n) {
  edge_gen.initialize(Graph::K, n);
  Counters::new_growth_step(n, canonicals[n - 1].size());

  // This array will be reused when processing the graphs.
  VertexSignature vs[MAX_VERTICES];

  for (const Graph* g : canonicals[n - 1]) {
    Counters::increment_growth_processed_graphs_in_current_step();
    edge_gen.reset_enumeration();
    assert(n == Graph::K || g->is_canonical);

    Graph* copy = allocator.get_current_graph_from_allocator();
    // Loop through all ((K+1)^\binom{n-1}{k-1} - 1) edge combinations, add them to g, and check
    // add to canonicals unless it's isomorphic to an existing one.
    while (edge_gen.next()) {
      g->copy(copy);
      for (int i = 0; i < edge_gen.edge_count; i++) {
        copy->add_edge(edge_gen.edges[i]);
      }
      if (copy->contains_Tk(n - 1)) {
        edge_gen.notify_contain_tk_skip();
        continue;
      }

      copy->canonicalize(vs);

      if (!canonicals[n].contains(copy)) {
        canonicals[n].insert(copy);
        Counters::observe_theta(*copy);
        Counters::current_set_stats(canonicals[n].bucket_count(), canonicals[n].load_factor(),
                                    canonicals[n].max_load_factor());
        allocator.mark_current_graph_used();
        copy = allocator.get_current_graph_from_allocator();
      }
    }
  }
}

void Grower::print_config(std::ostream& os) {
  os << "Searching for all T_k-free k-PDGs\n    K= " << Graph::K
     << " (number of vertices in each edge)\n    N= " << Graph::N
     << " (total number of vertices in each graph)\n    E= " << MAX_EDGES
     << " (maximum possible number of edges in each graph)\n";
}
void Grower::print_state_to_stream(bool print_graphs, std::ostream& os) {
  uint64 total_canonicals = 0;
  for (int i = 0; i < Graph::N + 1; i++) {
    os << "order=" << i << " : canonicals= " << canonicals[i].size() << "\n";
    total_canonicals += canonicals[i].size();
    if (print_graphs) {
      for (const Graph* g : canonicals[i]) {
        g->print_concise(os);
      }
    }
  }
  os << "Total canonicals= " << total_canonicals << "\n";
}
