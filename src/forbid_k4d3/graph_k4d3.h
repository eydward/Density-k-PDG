#pragma once

#include "../graph.h"

// Returns true if the given graph g contains F={01, 02, 03, 12>2, 13>3, 23>3}, and the given
// vertex v is a vertex in F. Only works for 2-PDGs (K=2).
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing undirected edges to directed edges.
bool contains_K4D3(Graph& g, int v);