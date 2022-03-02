#pragma once
#include <bits/stdc++.h>
using namespace std;

template <int N>
struct Permutator {
  // One or more ranges to permute. Each range is specified as [a,b) interval.
  // For example if perm_sets is {(4,6), (0,3)}, then the permutations generated will be
  // 01234567, 01235467, 02134566, 02135467, 10234567, 10235467, ...
  vector<pair<int, int>> perm_sets;

  // The current permutation state
  int p[N];

  Permutator(vector<pair<int, int>>&& psets) : perm_sets(psets) {
    // As a heuristic, we permute the shorter ranges first and longer ranges last.
    sort(perm_sets.begin(), perm_sets.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
      return a.second - a.first < b.second - b.first;
    });
    // Initialize the state
    for (int v = 0; v < N; v++) p[v] = v;
  }

  // Returns true if the next permutation is valid. Content in array p[N]
  // Caller should call this function in a loop until it returns false.
  bool next() {
    for (int i = 0; i < perm_sets.size(); i++) {
      bool valid = next_permutation(p + perm_sets[i].first, p + perm_sets[i].second);
      if (valid) return true;
    }

    return false;
  }
};