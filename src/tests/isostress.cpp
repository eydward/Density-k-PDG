// A stress test to verify the correctness of the isomorphism check algorithm.
// It's the most complex algorithm in this program so we want to use more than the unit tests
// and code review to confirm it's implemented correctly.
//
// The idea is to compare the result of Graph::is_isomorphic() and Graph::is_isomorphic_slow()
// and verify that they always match.
//
// Graph::is_isomorphic_slow() is a very simple bruteforce isomorphism check implementation,
// and we can easily verify its correctness by reviewing the code. So by using large number of
// graphs to verify Graph::is_isomorphic()==Graph::is_isomorphic_slow(), we gain high confidence
// that Graph::is_isomorphic() is implemented correctly.

#include "../counters.h"
#include "iso_stress_test.h"

int main(int argc, char* argv[]) {
  Counters::initialize();

  for (int diff = 0; diff <= 3; diff++) {
    for (int n = diff + 2; n <= 7; n++) {
      int k = n - diff;
      for (int c = 0; c < k; c++) {
        if (c == 1) continue;
        IsomorphismStressTest t(k, n, c);
        t.run();
      }
    }
  }
  std::cout << "\nALL DONE\n";
  Counters::print_counters();
  return 0;
}