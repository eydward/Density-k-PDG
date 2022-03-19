#include "edge_gen.h"

#include "counters.h"
constexpr uint8 NOT_IN_SET = 0xEE;

// Initializes the generator for the given new vertex count.
// k_value = number of vertices in each edge.
EdgeGenerator::EdgeGenerator(int k_value, int vertex_count)
    : k(k_value), n(vertex_count), edge_candidate_count(0), edge_count(0) {
  for (uint8 mask = 0; mask < (1 << (n - 1)); mask++) {
    if (__builtin_popcount(mask) == k - 1) {
      edge_candidates[edge_candidate_count] = (mask | (1 << (n - 1)));
      int vidx = 0;
      edge_candidates_vidx[edge_candidate_count][vidx++] = NOT_IN_SET;
      edge_candidates_vidx[edge_candidate_count][vidx++] = UNDIRECTED;
      for (uint8 i = 0; i <= n - 1; i++) {
        if ((edge_candidates[edge_candidate_count] & (1 << i)) != 0) {
          edge_candidates_vidx[edge_candidate_count][vidx++] = i;
        }
      }
      assert(vidx == k + 2);
      ++edge_candidate_count;
    }
  }
  assert(edge_candidate_count == compute_binom(n - 1, k - 1));
  reset_enumeration();
}

void EdgeGenerator::reset_enumeration() {
  edge_count = 0;
  high_idx_non_zero_enum_state = 0;
  for (uint8 i = 0; i < edge_candidate_count; i++) {
    enum_state[i] = 0;
  }
}

bool EdgeGenerator::next() {
  for (uint8 i = 0; i < edge_candidate_count; i++) {
    ++enum_state[i];
    high_idx_non_zero_enum_state = std::max(high_idx_non_zero_enum_state, i);
    if (enum_state[i] != k + 2) {
      // We found a new valid enumeration state. Collect info into edges array.
      uint32 current_candidate_mask = 0;
      edge_count = 0;
      for (uint8 j = 0; j <= high_idx_non_zero_enum_state; j++) {
        if (enum_state[j] != 0) {
          edges[edge_count].vertex_set = edge_candidates[j];
          edges[edge_count].head_vertex = edge_candidates_vidx[j][enum_state[j]];
          ++edge_count;
          current_candidate_mask |= (1 << j);
        }
      }
      return true;
    }
    enum_state[i] = 0;
  }
  return false;
}

// Notify the generator about the fact that adding the current edge set to the graph
// makes it contain T_k, and therefore we can skip edge sets that are supersets of the current.
void EdgeGenerator::notify_contain_tk_skip() {
  // Find the lowest non-zero enum state, and change everything below
  // it to the final state. Then the next() call will bump the lowest non-zero enum state.
  // For example, if the enum state is [3,0,0,1,0,0,0], update it to [3,0,0,1,k+1,k+1,k+1]
  // then the next call will get to [3,0,0,2,0,0,0].
  for (uint8 i = 0; i < edge_candidate_count; i++) {
    if (enum_state[i] != 0) return;
    enum_state[i] = k + 1;
  }
}

void EdgeGenerator::print_debug() const {
  std::cout << "EdgeGen[" << static_cast<int>(k) << ", " << static_cast<int>(n)
            << ", cand_count=" << static_cast<int>(edge_candidate_count)
            << ", high_idx=" << static_cast<int>(high_idx_non_zero_enum_state) << ",\n  EC={";
  for (int e = 0; e < edge_candidate_count; e++) {
    if (e > 0) std::cout << ", ";
    std::cout << std::bitset<8>(edge_candidates[e]);
  }
  std::cout << "}]\n";
}