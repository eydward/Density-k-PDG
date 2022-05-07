#pragma once

#include "../graph.h"

// Returns true if the given graph g contains F = {01, 02, 03, 12, 13, 23>3},
// namely a complete graph with 4 vertices and at least one directed edge, and the given
// vertex v is in F. Only works for 2-PDGs (K=3).
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing directed edges to undirected by forgetting directions.
bool contains_K4(const Graph& g, int v);