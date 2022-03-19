#include "edge_gen.h"

#include "counters.h"
constexpr uint8 NOT_IN_SET = 0xEE;

// Initializes the generator for the given new vertex count.
// k_value = number of vertices in each edge.
EdgeGenerator::EdgeGenerator(int k_value, int vertex_count, bool use_automorphism_optimization)
    : use_automorphism_opt(use_automorphism_optimization),
      k(k_value),
      n(vertex_count),
      edge_candidate_count(0),
      // When N<=7, the largest set of edge candidate is 6 choose 3 = 20.
      // So need to reserve 2^20 bits in this vector.
      edge_candidate_id_sets(1 << 20),
      edge_count(0) {
  for (int i = 0; i < 256; i++) {
    edge_to_id[i] = 0xFF;
  }

  for (uint8 mask = 0; mask < (1 << (n - 1)); mask++) {
    if (__builtin_popcount(mask) == k - 1) {
      edge_candidates[edge_candidate_count] = (mask | (1 << (n - 1)));
      edge_to_id[edge_candidates[edge_candidate_count]] = edge_candidate_count;
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
#if false
  // Populate the edge_candidate_id_sets, so that from 0b00...01 to 0b11...11 are all true,
  // indicating edge generation will try all combinations.
  for (int i = 1; i < (1 << edge_candidate_count); i++) {
    edge_candidate_id_sets[i] = true;
  }
#endif
}

bool EdgeGenerator::next() {
BEGIN:
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
#if false
      // If the current edge set is allowed, return true to use this set.
      // Otherwise jump back to the beginning and try the next state.
      if (edge_candidate_id_sets[current_candidate_mask]) {
        return true;
      } else {
        goto BEGIN;
      }
#endif
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

void EdgeGenerator::notify_automorphism(int perm[MAX_VERTICES]) {
  if (!use_automorphism_opt) return;
  return;
#if false
  for (int i = 1; i < (1 << edge_candidate_count); i++) {
    if (edge_candidate_id_sets[i]) {
      uint32 auto_id_set = 0;
      for (int e = 0; e < edge_candidate_count; e++) {
        if ((i & (1 << e)) != 0) {
          uint8 edge = edge_candidates[e];
          uint8 auto_edge = 0;
          for (int v = 0; v < Graph::N; v++) {
            if ((edge & (1 << v)) != 0) {
              auto_edge |= (1 << perm[v]);
              // std::cout << "v=" << v << ", p[v]=" << perm[v] << ", auto_e=" << (int)auto_edge
              //           << "\n";
            }
          }
          //          std::cout << "e=" << e << ", edge=" << (int)edge << ", auto_e=" <<
          //          (int)auto_edge << "\n";
          uint8 auto_id = edge_to_id[auto_edge];
          auto_id_set |= 1 << auto_id;
        }
      }

      // print_debug();
      // std::cout << "i=" << i << ", " << auto_id_set << ", c=" << (int)edge_candidate_count <<
      // "\n"; for (int e = 0; e < edge_candidate_count; e++) {
      //   std::cout << (int)edge_candidates[e] << ", ";
      // }
      // std::cout << "\n";
      // for (int v = 0; v < Graph::N; v++) {
      //   std::cout << perm[v] << ", ";
      // }
      // std::cout << "\n";

      assert(auto_id_set < (1 << edge_candidate_count));
      if (auto_id_set != i && edge_candidate_id_sets[auto_id_set]) {
        edge_candidate_id_sets[auto_id_set] = false;
        Counters::increment_growth_automorphisms_vset_skips();
      }
    }
  }
#endif
}

void EdgeGenerator::print_debug() const {
  int count = 0;
  for (int i = 0; i < (1 << edge_candidate_count); i++) {
    if (edge_candidate_id_sets[i]) count++;
  }

  std::cout << "EdgeGen[" << static_cast<int>(k) << ", " << static_cast<int>(n)
            << ", cand_count=" << static_cast<int>(edge_candidate_count)
            << ", high_idx=" << static_cast<int>(high_idx_non_zero_enum_state)
            << ", set count=" << count << ",\n  EC={";
  for (int e = 0; e < edge_candidate_count; e++) {
    if (e > 0) std::cout << ", ";
    std::cout << std::bitset<8>(edge_candidates[e]);
  }
  std::cout << "}\n  E->I={";
  int high_idx_edge_to_id = 255;
  for (; high_idx_edge_to_id >= 0; high_idx_edge_to_id--) {
    if (edge_to_id[high_idx_edge_to_id] != 0xFF) break;
  }
  for (int i = 0; i <= high_idx_edge_to_id; i++) {
    if (i % 8 == 0) {
      std::cout << "\n    [" << i << "] ";
    } else {
      std::cout << ", ";
    }
    if (edge_to_id[i] == 0xFF) {
      std::cout << " ";
    } else {
      std::cout << static_cast<int>(edge_to_id[i]);
    }
  }

  std::cout << "\n  }]\n";
}