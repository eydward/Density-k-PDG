#pragma once
#include "counters.h"
#include "graph.h"

// An extremely simple custom memory allocator to allocate Graph objects. Because all Graphs we
// add to the canonicals sets must stay alive forever, there is no reason to worry about freeing
// them, so we don't want to deal with the overhead of C++ runtime memory allocation.
//
// The motivation to use this, instead of letting the canonicals sets directly hold Graph
// objects (instead of Graph pointers), is because of the bad rebalancing behavior
// observed in std::unordered_set<G> when running large compute (K=4, N=7), when
// std::unordered_set<G> have to hold >100 million items and many GB of memory.
class GraphAllocator {
  class AllocationChunk {
   private:
    // Max number of items this allocator can hold. Use 4MB chunks. The "-128" part is to
    // allow bookkeeping overhead of C++ runtime heap, to stay below 4MB.
    static constexpr int MAX_ITEMS = ((1 << 22) - 128) / sizeof(Graph);
    Graph array[MAX_ITEMS];
    int current_index;

   public:
    AllocationChunk() : current_index(0) {}
    // Returns true if this allocator is full and cannot allocate anymore.
    bool is_full() const { return current_index == MAX_ITEMS; }
    // Mark the current graph object is used, advance to allocate the next object.
    // Caller must make sure it's not full before calling.
    void mark_current_used() {
      assert(!is_full());
      ++current_index;
    }
    // Returns the current graph object. Caller must make sure it's not full before calling.
    Graph* get_current_graph() {
      assert(!is_full());
      array[current_index].clear();
      return array + current_index;
    }
  };

  // The current memory allocator.
  AllocationChunk* chunk;

 public:
  GraphAllocator() : chunk(nullptr) {}

  Graph* get_current_graph_from_allocator() {
    if (chunk == nullptr || chunk->is_full()) {
      chunk = new AllocationChunk();
      Counters::increment_chunk_allocations();
    }
    return chunk->get_current_graph();
  }
  void mark_current_graph_used() {
    assert(chunk != nullptr);
    Counters::increment_graph_allocations();
    return chunk->mark_current_used();
  }
};