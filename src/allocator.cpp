#include "allocator.h"

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

Graph* GraphAllocator::get_current_graph_from_allocator() {
  if (chunk == nullptr || chunk->is_full()) {
    chunk = new AllocationChunk();
    Counters::increment_chunk_allocations();
  }
  return chunk->get_current_graph();
}
void GraphAllocator::mark_current_graph_used() {
  assert(chunk != nullptr);
  Counters::increment_graph_allocations();
  return chunk->mark_current_used();
}
