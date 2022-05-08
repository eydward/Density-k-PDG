#pragma once

#include "../graph.h"
#include "../grower.h"

// Returns true if the graph contains the generalized triangle T_k as a subgraph, where
// v is one of the vertices of the T_k subgraph.
// T_k is defined as (K+1)-vertex, 3-edge K-graph, with two undirected edges and one directed
// edge, where all edges share the same set of vertices except for {1,2,3}.
// For example T_2={12, 13, 23>3}, T_3={124, 134, 234>3}, T_4={1245, 1345, 2345>3}, etc.
//
// Subgraph definition: F is a subgraph of G iff F can be obtained from G by
// (1) deleting vertices
// (2) deleting edges
// (3) changing directed edges to undirected by forgetting directions.
bool contains_Tk(const Graph& g, int v);

// Derived class to manage the growth search, for T_k-free problem.
class GrowerTk : public Grower {
  using Grower::Grower;
  Fraction get_ratio(const Graph& g) const override { return g.get_theta_ratio(); }
  bool contains_forbidden_subgraph(Graph& g, int v) const override { return contains_Tk(g, v); }
};