#pragma once
#include "counters.h"
#include "graph.h"

// Forward declaration of some implemetation detail in the cpp file.
class AllocationChunk;

// An extremely simple custom memory allocator to allocate Graph objects. Because all Graphs we
// add to the canonicals sets must stay alive forever, there is no reason to worry about freeing
// them, so we don't want to deal with the overhead of C++ runtime memory allocation.
//
// The motivation to use this, instead of letting the canonicals sets directly hold Graph
// objects (instead of Graph pointers), is because of the bad rebalancing behavior
// observed in std::unordered_set<G> when running large compute (K=4, N=7), when
// std::unordered_set<G> have to hold >100 million items and many GB of memory.
class GraphAllocator {
 private:
  // The current memory allocator.
  AllocationChunk* chunk;

 public:
  GraphAllocator();

  // Returns a usable graph object with everything zeroed. If the caller
  // wants to keep the object (e.g. store it in some C++ container),
  // it must call mark_current_graph_used(), otherwise the next call
  // to get_current_graph_from_allocator() will return the same object.
  Graph* get_current_graph_from_allocator();

  // Marks the current object as used, advance the allocator state, so that
  // the next call get_current_graph_from_allocator() will return a new object.
  void mark_current_graph_used();
};