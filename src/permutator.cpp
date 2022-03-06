#include "permutator.h"

Permutator::Permutator(std::vector<std::pair<int, int>>&& psets) : perm_sets(psets) {
  // As a heuristic, we permute the shorter ranges first and longer ranges last.
  std::sort(perm_sets.begin(), perm_sets.end(),
            [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
              return a.second - a.first < b.second - b.first;
            });
  // Initialize the state
  for (int v = 0; v < MAX_VERTICES; v++) p[v] = v;
}

bool Permutator::next() {
  for (int i = 0; i < perm_sets.size(); i++) {
    bool valid = std::next_permutation(p + perm_sets[i].first, p + perm_sets[i].second);
    if (valid) return true;
  }

  return false;
}
