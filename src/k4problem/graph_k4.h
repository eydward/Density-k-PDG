#pragma once

#include "../graph.h"
#include "../grower.h"

// Returns true if the given graph g contains F = {01, 02, 03, 12, 13, 23>3},
// namely a complete graph with 4 vertices and at least one directed edge, and the given
// vertex v is in F. Only works for 2-PDGs (K=3).
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing directed edges to undirected by forgetting directions.
bool contains_K4(const Graph& g, int v);

// Derived class to manage the growth search, for K4-free problem.
class GrowerK4 : public Grower {
  using Grower::Grower;
  Fraction get_ratio(const Graph& g) const override { return g.get_theta_ratio(); }
  bool contains_forbidden_subgraph(Graph& g, int v) const override { return contains_K4(g, v); }
};