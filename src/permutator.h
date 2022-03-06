#pragma once
#include <bits/stdc++.h>

struct Permutator {
  // One or more ranges to permute. Each range is specified as [a,b) interval.
  // For example if perm_sets is {(4,6), (0,3)}, then the permutations generated will be
  // 01234567, 01235467, 02134566, 02135467, 10234567, 10235467, ...
  std::vector<std::pair<int, int>> perm_sets;

  // The current permutation state
  static constexpr int MAX_VERTICES = 7;
  int p[MAX_VERTICES];

  Permutator(std::vector<std::pair<int, int>>&& psets) : perm_sets(psets) {
    // As a heuristic, we permute the shorter ranges first and longer ranges last.
    std::sort(perm_sets.begin(), perm_sets.end(),
              [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                return a.second - a.first < b.second - b.first;
              });
    // Initialize the state
    for (int v = 0; v < MAX_VERTICES; v++) p[v] = v;
  }

  // Returns true if the next permutation is valid. Content in array p[N]
  // Caller should call this function in a loop until it returns false.
  bool next() {
    for (int i = 0; i < perm_sets.size(); i++) {
      bool valid = std::next_permutation(p + perm_sets[i].first, p + perm_sets[i].second);
      if (valid) return true;
    }

    return false;
  }
};