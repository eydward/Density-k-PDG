#pragma once
#include <bits/stdc++.h>

// A simple utility that generates permutations on the specified ranges.
// See the comment on the constructor for details.
class Permutator {
 private:
  std::vector<std::pair<int, int>> perm_sets;

  // The current permutation state
  static constexpr int MAX_VERTICES = 7;

 public:
  // The current permutation state. Valid after each next() call that returns true.
  int p[MAX_VERTICES];

  // Creates a Permutator object, initialized with the given permutation sets.
  //
  // permutation_sets:
  // One or more ranges to permute. Each range is specified as [a,b) interval.
  // For example if permutation_sets is {(4,6), (0,3)}, then the permutations
  // generated will be:
  //    01234567, 01235467, 02134566, 02135467, 10234567, 10235467, ...
  Permutator(std::vector<std::pair<int, int>>&& permutation_sets);

  // Returns true if the next permutation is valid. Content in array p[N]
  // Caller should call this function in a loop until it returns false.
  bool next();
};