#pragma once

#include "../graph.h"
#include "../grower.h"

// Returns true if the given graph g contains F={01, 02, 03, 12>2, 13>3, 23>3}, and the given
// vertex v is a vertex in F. Only works for 2-PDGs (K=2).
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing undirected edges to directed edges.
bool contains_K4D3(Graph& g, int v);

// Derived class to manage the growth search, for K4D3-free problem.
class GrowerK4D3 : public Grower {
  using Grower::Grower;  // Inherite the constructor from the base class.
  Fraction get_ratio(const Graph& g) const override { return g.get_zeta_ratio(); }
  bool contains_forbidden_subgraph(Graph& g, int v) const override { return contains_K4D3(g, v); }
};