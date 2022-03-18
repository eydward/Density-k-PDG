#include "grower.h"

#include "counters.h"

Grower::Grower(bool print_graph_, std::ostream* log_stream)
    : print_graph(print_graph_), log(log_stream) {}

// Find all canonical isomorphism class representations with up to max_n vertices.
void Grower::grow() {
  assert(Graph::N <= 7);
  print_config(std::cout);
  if (log != nullptr) {
    print_config(*log);
  }

  // Initialize empty graph with k-1 vertices.
  Graph g;
  g.canonicalize();
  canonicals[Graph::K - 1].insert(g);
  Counters::observe_theta(g);

  // First grow to N-1 vertices, accumulate one graph from each isomorphic class.
  for (int n = Graph::K; n < Graph::N; n++) {
    grow_step(n);
  }
  Counters::print_counters();
  print(print_graph);
  // Finally, enumerate all graphs with N vertices, no need to store graphs.
  enumerate_final_step();
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
  assert(n < Graph::N);
  edge_gen.initialize(Graph::K, n);
  Counters::new_growth_step(n, canonicals[n - 1].size());

  // This data structure will be reused when processing the graphs.
  Graph copy;

  for (const Graph& g : canonicals[n - 1]) {
    Counters::increment_growth_processed_graphs_in_current_step();
    edge_gen.reset_enumeration();

#if false
    int perm[MAX_VERTICES];
    for (int v = 0; v < MAX_VERTICES; v++) perm[v] = v;
    Graph h;
    while (std::next_permutation(perm, perm + n - 1)) {
      g->permute_canonical(perm, h);
      if (g->is_identical(h)) {
        Counters::increment_growth_automorphisms_found();
        edge_gen.notify_automorphism(perm);
      }
    };
#endif

    // Loop through all ((K+1)^\binom{n-1}{k-1} - 1) edge combinations, add them to g, and check
    // add to canonicals unless it's isomorphic to an existing one.
    while (edge_gen.next()) {
      g.copy_edges(copy);
      for (int i = 0; i < edge_gen.edge_count; i++) {
        copy.add_edge(edge_gen.edges[i]);
      }
      if (copy.contains_Tk(n - 1)) {
        edge_gen.notify_contain_tk_skip();
        continue;
      }

      copy.canonicalize();

      if (!canonicals[n].contains(copy)) {
        canonicals[n].insert(copy);
        Counters::observe_theta(copy);
      }
    }
  }
}

void Grower::enumerate_final_step() {
  edge_gen.initialize(Graph::K, Graph::N);
  Counters::enter_final_step(canonicals[Graph::N - 1].size());

  // This data structure will be reused when processing the graphs.
  Graph copy;

  for (const Graph& g : canonicals[Graph::N - 1]) {
    Counters::increment_growth_processed_graphs_in_current_step();
    edge_gen.reset_enumeration();
    while (edge_gen.next()) {
      g.copy_edges(copy);
      for (int i = 0; i < edge_gen.edge_count; i++) {
        copy.add_edge(edge_gen.edges[i]);
      }
      if (copy.contains_Tk(Graph::N - 1)) {
        edge_gen.notify_contain_tk_skip();
        continue;
      }

      Counters::observe_theta(copy);
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
  os << "Growth phase completed. State:\n";
  uint64 total_canonicals = 0;
  for (int i = 0; i < Graph::N; i++) {
    os << "  order=" << i << " : canonicals= " << canonicals[i].size() << "\n";
    total_canonicals += canonicals[i].size();
    if (print_graphs) {
      for (const Graph& g : canonicals[i]) {
        os << "    ";
        g.print_concise(os);
      }
    }
  }
  os << "Total canonicals= " << total_canonicals << "\nStarting final enumeration phase...\n";
}
