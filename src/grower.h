#pragma once

#include "graph.h"

// Grow set of non-isomorphic graphs from empty graph, by adding one vertex at a time.
template <int K, int N>
class Grower {
 private:
  // Alias of the Graph type
  typedef Graph<K, N> G;

  // Custom hash and compare for the Graph type. Treat isomorphic graphs as being equal.
  struct GraphHasher {
    size_t operator()(const G& g) const { return g.hash; }
  };
  struct GraphComparer {
    bool operator()(const G& g, const G& h) const { return g.is_isomorphic(h); }
  };

  // The log file.
  ofstream* const log;

  // Constructs all non-isomorphic graphs with n vertices that are T_k-free,
  // and add them to the canonicals. Before calling this, all such graphs
  // with <n vertices must already be in the canonicals.
  // Note all edges added in this step contains vertex (n-1).
  void grow_step(int n) {
    for (const G& g : canonicals[n - 1]) {
      assert(n == K || (g.is_canonical && g.vertex_count == n - 1));
      assert(g.vertices[n - 1].get_degrees() == 0);

      // First find all edge candidates through the new vertex.
      vector<uint8> edge_candidates;
      for (uint8 mask = 0; mask < (1 << (n - 1)); mask++) {
        if (__builtin_popcount(mask) == K - 1) {
          edge_candidates.push_back(mask | (1 << (n - 1)));
        }
      }

      // Add one edge at a time:
      // (1) current = {g}, next = empty.
      // (2) for each graph in current, for each edge candidate, try to add. If can, put in next.
      // (3) Add everything in current to canonicals, and move next to current.
      // Repeat for m times.
      unordered_set<G, GraphHasher, GraphComparer> current;
      current.insert(g);
      unordered_set<G, GraphHasher, GraphComparer> next;

      G copy;
      for (int i = 0; i < edge_candidates.size(); i++) {
        for (const G& start : current) {
          for (const uint8 edge : edge_candidates) {
            if (start.edge_allowed(edge)) {
              for (int head = -1; head < n; head++) {
                if (head >= 0 && (edge & (1 << head)) == 0) continue;
                start.copy_without_init(copy);
                copy.add_edge(edge, head < 0 ? UNDIRECTED : head);
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

  void print_config(ostream& os) {
    os << "Searching for all T_k-free k-PDGs\n    K= " << K
       << " (number of vertices in each edge)\n    N= " << N
       << " (total number of vertices in each graph)\n    E= " << G::MAX_EDGES
       << " (maximum possible number of edges in each graph)\n";
  }
  void print_state_to_stream(bool print_graphs, ostream& os) {
    for (int i = 0; i < N + 1; i++) {
      os << "order=" << i << " : # canonicals=" << canonicals[i].size() << "\n";
      if (print_graphs) {
        for (const G& g : canonicals[i]) {
          g.print_concise(os);
        }
      }
    }
  }

 public:
  Grower(ofstream* log_stream = nullptr) : log(log_stream) {}

  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  unordered_set<G, GraphHasher, GraphComparer> canonicals[N + 1];

  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow() {
    static_assert(N <= 8);
    print_config(cout);
    if (log != nullptr) {
      print_config(*log);
    }

    // Initialize empty graph with k-1 vertices.
    G g;
    g.init();
    canonicals[K - 1].insert(g);

    for (int n = K; n <= N; n++) {
      grow_step(n);
    }
  }

  // Debug print the content of the canonicals after the growth.
  // If print_graphs==true, print stats and all graphs. Otherwise prints stats only.
  void print(bool print_graphs) {
    print_state_to_stream(print_graphs, cout);
    if (log != nullptr) {
      print_state_to_stream(print_graphs, *log);
    }
  }
};