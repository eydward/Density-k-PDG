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

  // One canonical graphs with n vertices in each isomorphism class is in canonicals[n].
  unordered_set<G, GraphHasher, GraphComparer> canonicals[N];

  void grow_step(int n) {
    for (const G& g : canonicals[n]) {
    }
  }

 public:
  // Find all canonical isomorphism class representations with up to max_n vertices.
  void grow(int max_n) {
    // Initialize empty graph with k-1 vertices.
    G g;
    g.init();
    canonicals[K - 1].insert(g);

    for (int n = K - 1; n < max_n; n++) {
      grow_step(n);
    }
  }
};