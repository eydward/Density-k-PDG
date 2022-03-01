#include "graph.h"

// Grow set of non-isomorphic graphs from empty graph, by adding one vertex at a time.
template <int K, int N, int MAX_EDGES>
class Grower {
  typedef Graph<K, N, MAX_EDGES> G;

  struct GraphHasher {
    size_t operator()(const G& g) const { return g.hash; }
  };
  struct GraphComparer {
    bool operator()(const G& g, const G& h) const { return g.is_isomorphic(h); }
  };

  // Constructs all non-isomorphic graphs with n vertices, and add them to the canonicals.
  // Before calling this, all such graphs with <n vertices must already be in the canonicals.
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

      G copy, canonical;
      for (int i = 0; i < edge_candidates.size(); i++) {
        for (const G& start : current) {
          for (const uint8 edge : edge_candidates) {
            if (start.edge_allowed(edge)) {
              for (int head = -1; head < n; head++) {
                if (head >= 0 && (edge & (1 << head)) == 0) continue;
                start.copy_without_init(copy);
                copy.add_edge(edge, head < 0 ? UNDIRECTED : head);
                copy.init();
                copy.canonicalize(canonical);

                if (!canonicals[n].contains(canonical) && !current.contains(canonical) &&
                    !next.contains(canonical)) {
                  next.insert(canonical);
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

 public:
  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  unordered_set<G, GraphHasher, GraphComparer> canonicals[N];

  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow(int max_n) {
    // Initialize empty graph with k-1 vertices.
    G g;
    g.init();
    canonicals[K - 1].insert(g);

    for (int n = K; n <= max_n; n++) {
      grow_step(n);
    }
  }

  // Debug print the content of the canonicals after the growth.
  void print() {
    cout << dec;
    for (int i = 0; i < N; i++) {
      cout << "order=" << i << " : # canonicals=" << canonicals[i].size() << "\n";
      for (const G& g : canonicals[i]) {
        g.print_concise();
      }
    }
  }
};