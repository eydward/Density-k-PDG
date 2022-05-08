// A stress test to verify the correctness of the edge generation algorithm.
//
// The idea is to compare the result of Grower.grow(), using different edge gen optimization
// combinations, and verify that they always match.
//
#include "../counters.h"
#include "../forbid_tk/graph_tk.h"

#define ASSERT(c)                         \
  if (!(c)) {                             \
    std::cout << "Failed assert: " << #c; \
    exit(-1);                             \
  }

void verify_array_equal(const std::vector<std::tuple<int, Graph, Graph>>& a,
                        const std::vector<std::tuple<int, Graph, Graph>>& b) {
  ASSERT(a.size() == b.size());
  for (size_t i = 0; i < a.size(); i++) {
    ASSERT(std::get<0>(a[i]) == std::get<0>(b[i]));
    ASSERT(std::get<1>(a[i]).serialize_edges() == std::get<1>(b[i]).serialize_edges());
    ASSERT(std::get<2>(a[i]).serialize_edges() == std::get<2>(b[i]).serialize_edges());
  }
}
void verify(int k, int n) {
  Graph::set_global_graph_info(k, n);

  Counters::initialize();
  GrowerTk ff(0, false, false, false, 0, 0);
  ff.grow();

  Counters::initialize();
  GrowerTk ft(0, false, false, true, 0, 0);
  ft.grow();

  Counters::initialize();
  GrowerTk tf(0, false, true, false, 0, 0);
  tf.grow();

  Counters::initialize();
  GrowerTk tt(0, false, true, true, 0, 0);
  tt.grow();

  verify_array_equal(ff.get_results(), ft.get_results());
  verify_array_equal(ff.get_results(), tf.get_results());
  verify_array_equal(ff.get_results(), tt.get_results());
}

int main(int argc, char* argv[]) {
  for (int n = 2; n <= 7; n++) {
    verify(2, n);
  }
  for (int k = 3; k <= 7; k++) {
    verify(k, k);
  }
  for (int k = 3; k <= 6; k++) {
    verify(k, k + 1);
  }
  for (int k = 3; k <= 5; k++) {
    verify(k, k + 2);
  }
  verify(3, 6);

  std::cout << "\n\nDone: all successful.\n";
  return 0;
}