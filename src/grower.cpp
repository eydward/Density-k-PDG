#include "grower.h"

#include "counters.h"
#include "fraction.h"

Grower::Grower(int num_worker_threads_, int restart_idx_, std::ostream* log_,
               std::ostream* log_detail_)
    : num_worker_threads(num_worker_threads_),
      restart_idx(restart_idx_),
      log(log_),
      log_detail(log_detail_),
      to_be_processed_id(0) {}

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
  print_before_final();
  // Finally, enumerate all graphs with N vertices, no need to store graphs.
  enumerate_final_step();
}

// Constructs all non-isomorphic graphs with n vertices that are T_k-free,
// and add them to the canonicals. Before calling this, all such graphs
// with <n vertices must already be in the canonicals.
// Note all edges added in this step contains vertex (n-1).
void Grower::grow_step(int n) {
  assert(n < Graph::N);
  EdgeGenerator edge_gen(Graph::K, n);
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
  Counters::enter_final_step(canonicals[Graph::N - 1].size());
  for (const Graph& g : canonicals[Graph::N - 1]) {
    to_be_processed.push(g);
  }
  if (restart_idx > 0) {
    for (int i = 0; i < restart_idx; i++) {
      to_be_processed.pop();
      ++to_be_processed_id;
    }
  }

  if (num_worker_threads == 0) {
    worker_thread_main(0);
  } else {
    // Start the threads.
    for (int i = 0; i < num_worker_threads; i++) {
      worker_threads.push_back(std::thread(worker_thread_main, this, i + 1));
    }
    // Wait for them to finish.
    for (std::thread& t : worker_threads) {
      t.join();
    }
  }
}

void Grower::worker_thread_main(int thread_id) {
  EdgeGenerator edge_gen(Graph::K, Graph::N);
  // These instances will be reused when processing the graphs.
  Graph base;
  int base_graph_id;
  Graph copy;
  Graph min_theta_graph;

  while (true) {
    // The lock scope to safely get a graph from the queue.
    {
      std::scoped_lock lock(queue_mutex);
      if (to_be_processed.empty()) return;
      base = to_be_processed.front();
      base_graph_id = to_be_processed_id++;
      to_be_processed.pop();
      Counters::increment_growth_processed_graphs_in_current_step();
    }

    Fraction min_theta(1E8, 1);
    uint64 graphs_processed = 0;
    edge_gen.reset_enumeration();
    while (edge_gen.next()) {
      base.copy_edges(copy);
      for (int i = 0; i < edge_gen.edge_count; i++) {
        copy.add_edge(edge_gen.edges[i]);
      }
      if (copy.contains_Tk(Graph::N - 1)) {
        edge_gen.notify_contain_tk_skip();
        continue;
      }

      // Bookkeeping: retain the minimum theta value encountered so far, and the graph genreated it.
      graphs_processed++;
      if (copy.get_theta() < min_theta) {
        min_theta = copy.get_theta();
        min_theta_graph = copy;
      }
    }

    // The lock scope to add the min theta to the global Counters.
    {
      std::scoped_lock lock(counters_mutex);
      Counters::observe_theta(min_theta_graph, graphs_processed);
      if (log_detail != nullptr) {
        *log_detail << "---- T[" << thread_id << "][" << base_graph_id
                    << "] G: min_theta = " << min_theta.n << " / " << min_theta.d << " :\n  ";
        base.print_concise(*log_detail);
        *log_detail << "  ";
        min_theta_graph.print_concise(*log_detail);
        log_detail->flush();
      }
    }
  }
}

// Print the content of the canonicals after the growth to console and log files.
void Grower::print_before_final() const {
  print_state_to_stream(std::cout);
  if (log != nullptr) {
    print_state_to_stream(*log);
    log->flush();
  }
  if (log_detail != nullptr) {
    for (int i = 0; i < Graph::N; i++) {
      *log_detail << "-------- Accumulated canonicals[order=" << i << "] : " << canonicals[i].size()
                  << " --------\n";
      int idx = 0;
      for (const Graph& g : canonicals[i]) {
        *log_detail << "  [" << idx++ << "] ";
        g.print_concise(*log_detail);
      }
    }
    log_detail->flush();
  }
}
void Grower::print_config(std::ostream& os) const {
  os << "Searching for all T_k-free k-PDGs\n    K= " << Graph::K
     << " (number of vertices in each edge)\n    N= " << Graph::N
     << " (total number of vertices in each graph)\n    E= " << MAX_EDGES
     << " (maximum possible number of edges in each graph)\n";
}
void Grower::print_state_to_stream(std::ostream& os) const {
  os << "Growth phase completed. State:\n";
  uint64 total_canonicals = 0;
  for (int i = 0; i < Graph::N; i++) {
    os << "  order=" << i << " : canonicals= " << canonicals[i].size() << "\n";
    total_canonicals += canonicals[i].size();
  }
  os << "Total canonicals= " << total_canonicals << "\nStarting final enumeration phase...\n";
}
