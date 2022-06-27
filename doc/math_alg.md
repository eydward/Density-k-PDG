# Math, Data Structure, and Algorithms

*This document outlines the mathematical background, data structure, and algorithms
used in the code. Use the [Development](./develop.md) doc for information about development
environment setup and how to contribute.*

## Introduction
This program computes the `min_ratio` value for k-PDGs with a fixed number of vertices and some forbidden sub-structures.

![IMAGE](./kpdg_intro.png?raw=true)

## Results
Values in the table are <img src="./theta_kn.png" height="22" /> for the various `K,N` combinations.

| N   | K=2 | K=3 | K=4 | K=5 | K=6 | K=7 | K=8 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2   | 1   |     |     |     |     |     |     |
| 3   | 3/2 | 1   |     |     |     |     |     |
| 4   | 3/2 | 4/3 | 1   |     |     |     |     |
| 5   | 5/3 | 5/3 | 5/4 | 1   |     |     |     |
| 6   | 5/3 | 5/3 | 3/2 | 6/5 | 1   |     |     |
| 7   | 7/4 | 7/4 | 7/4 | 7/5 | 7/6 | 1   |     |
| 8   | 7/4 | ?   | ?   | 8/5 | 4/3 | 8/7 | 1   |
| 9   | 9/5 |     |     |     |     | 9/7 | 9/8 |

All results shown in this table can be obtained very fast using an optimized build, except
* `K=5, N=7` takes a few minutes on a regular computer
* `K=6,N=8` took 200 CPU hours (log files from the run in `results/K6N8` directory).
* `K=3, N=7` took 880 CPU hours (logs files from the run in `results/K3N7` directory).
* `K=4, N=7` took almost 6600 CPU hours in total on Google Cloud, in 30 batches. The log files are in the `results/K4N7` directory. The `collector` utility is used to verify the consistency of all log files to summarize the final result. The graph that produced `theta_ratio=7/4` value is the following (note it's symmetric over {5,6}, making it easy to verify by hand).
    ```
    {
      0123>0, 0124>0, 0134>0, 0234>0, 0125>0, 0135>0, 0235>0, 0145>0, 0245>0, 0345>0,
      0126>0, 0136>0, 0236>0, 0146>0, 0246>0, 0346>0, 0156>0, 0256>0, 0356>0, 0456>0
    }
    {
      0125>5, 0135>5, 0235>5, 1235>5, 0145>5, 0245>5, 1245>5, 0345>5, 1345>5, 2345>5, 
      0126>6, 0136>6, 0236>6, 1236>6, 0146>6, 0246>6, 1246>6, 0346>6, 1346>6, 2346>6
    }
    ```
* `K=5,N=8` is not computationally feasible with the current approach. The final enumeration phase has 4722759 base graphs to compute (whereas in the `K=4,N=7` case, there are 29313 base graphs). A quick experiment suggests that processing each base graph will likely take >10^8 CPU hours which implies hueristically that it could take >10^15 CPU hours to compute larger cases, like `K=5,N=8`.

## Running the program
The easiest way to run is to get the binaries from either `bin-linux` or `bin-windows` directories. Alternatively, you can build from source: see the next section for details. To run the program:

`  kPDG K N T`

where `K`, `N` are defined as above, and `T` is the number of threads in the final enumeration step (for best performance, use the number of CPUs on the computer). If the program will run for a long time and there is a need to distribute the work to multiple computers, use

`  kPDG K N T start_idx end_idx`

where `start_idx` and `end_idx` specify the range of base graph ids in the final enumeration phase (both inclusive). For example for `K=4, N=7`, the final enumeration phase has 29313  base graphs. We can distribute this to 30 machines, with 

```
   kPDG K N T 0 1000
   kPDG K N T 1000 2000
   ... 
   kPDG K N T 29000 30000
```
Note that `end_idx` is allowed to be larger than the number of available base graphs for convenience. Also, in the example given, graphs with id 1000, 2000, etc. are actually processed twice, which is fine. To see how many base graphs there are, just run the program with `kPDG K N -1` which skips the final step and therefore runs relatively quickly, and observe the output saying `Growth phase completed. State:` and take the last `collected` value. 

### Ratio-graph search (requires v10 build or later)
Once we have calculated the min_ratio value, we can run the program to find all graphs that produce the min_ratio. In fact this is much faster because the min_ratio optmization (explained below) becomes much more effective. For example, running `k=3,n=7` combination normally to find min_ratio takes around 880 CPU hours. But once we know its min_ratio=7/4, finding all graphs that produces theta_ratio=7/4 (and verifying that indeed there are no graphs with theta value less than 7/4) only takes ~2 CPU hours. The command line argument to run the ratio-graph search is
`  kPDG K N T start_idx end_idx ratio_numerator ratio_denominator`

For example, to search for all graphs producing theta_ratio=7/4 for `k=3,n=7` using 10 threads, use `kPDG 3 7 10 0 0 7 4`. Note the arguments ` 0 0 ` are the start/end index of the base graphs, and 0s means search all base graphs. The numerical results and some very special cases that can be checked by hand suggest that the following might be true: when `K<=N<2K`, the min_theta_ratio is `N/K`. And when `N=2K`, the min_theta_ratio is `(2K-1)/K`. This is verified for all computationally feasible `K,N` combinations in the table, the results are in the `results/ExactGraphs` directory, with the exhaustive list of graphs in `*_result.log`. 


## Testing and Verification
We use the following steps to verify the correctness of the algorithm:
* The min_ratio values for `K=2` in the table are mathematically proved for all `N`.
* The min_ratio values for `N=K+1` cells in the table are mathemaically proved.
* The min_ratio value for `K=3, N=5` in the table is mathematically proved.
* Unit tests in `tests` directory covers all code in the program. See [instructions](src/tests/README.md) for details to check test coverage.
* Isomorphism stress test (`isostress`): use a straightforward `is_isomorphic_slow()` implementation without any optimization, and compare its result against `is_isomorphic()` which contains various optimizations. For `(K,N)` combinations that yield relatively small number of graphs, do this on all graph pairs exhaustively. Otherwise sample the graph pairs randomly to perform this check.
* Edge generator stress test (`edgegenstress`): There are two optimizations we implemented in the edge generation (explained in details below). In order to verify the optimizations are correct, this stress runs all combinations of `K, N` values where `N<=7`, except the two very slow ones: `K=3,N=7` and `K=4,N=7`. For each `K,N` combination, we execute the full growth search, using four different optimization combinations (`(false,false)`, `(false,true)`, `(true,false)`,`(true,true)`,), and compare their resulting graphs, verify everything is identical regardless of the edge_gen optimizations used. 


## Algorithm Design Summary
### Growing the Search Tree
We use a simple idea to grow the search tree while trying to avoid unnecessary work where we can. This is implemented in `grower.h, .cpp` with the edge set enumeration logic implemented in `edge_gen.h, .cpp`.  Note all graphs are on N vertices `{0,1,...,N-1}`. When we say "a graph with n vertices" in this section, we mean the number of vertices that have positive degrees.

1. Begin with `K-1` vertices `{0,...,K-2}` and 0 edges. This is the only "graph" in `collected_graphs[K-1]` (defined in `edge_gen.cpp`, `grow()` function).
2. Repeat for `n in {K, K+1, ..., N-1}`:
    - Use `collected_graphs[n-1]` as the base graph set.
    - Initialize the resulting graph set be base graph set.
    - Add vertex `n-1` to the vertex set of each candidate graph in the base graph set.
    - General all edge candidates that contain the new vertex, by checking all `\binom{n-1}{k-1}` vertex sets without vertex `n-1`, and adding vertex `n-1` to each edge. (This is in `EdgeCandidates` constructor in `edge_gen.cpp`.)
    - For each graph `g` in the base graph set, we want to add all possible combinations of the edge candidates (`(K+2)^\binom{n-1}{k-1}-1` in total) to `g` and see if we can generate additional graphs that are not isomorphic to anything already in the canonical set. This is done in `next()` in `edge_gen.cpp`. Essentially, we run a `\binom{n-1}{k-1}` digit counter where the value of each digit goes from `0` to `K+1`. `0` means the corresponding edge is not in the set to be added to `g`. `1` means the corresponding edge is in the set to be added to `g`, as an undirected edge. `>=2` means the corresponding edge is in the set to be added to `g`, as a directed edge, with the value indicating the head. For example if the edge candidates has `6` edges, then we run a 6-digit counter: `000001`, `000002`, ... , `00000(K+1)`, `000010`, `000011`, ... `(K+1)(K+1)(K+1)(K+1)(K+1)(K+1)`. And each counter value corresponds to a set of edges to be added to `g`.
        * For each edge set: make a copy of `g`, add the edge set to the copy, check whether it's `T_k`-free (explained below). 
        * If it contains `T_k` as a subgraph, ignore it and move on to the next edge set from the edge generator.
        * If it's `T_k`-free, canonicalize it (explained below), and check if it is isomorphic to some graph already stored in the resulting graph set. If it's not isomorphic to any existing graph, add it to the resulting graph set. Here we also check if the new graph gives us a smaller theta_ratio value than previously observed, if so remembers the theta_ratio value, and the graph that produced it. 
    - After the enumeration above is done, sort the resulting graph set, and it becomes `collected_graphs[n]`. 
3. Now we have accumulated one graph in each isomorphism class for graphs with `N-1` vertices. Start the final enumeration phase. This is essentially same as the previous step above, except for the fact that we don't need to store any generated graph, therefore there is no need to either canonicalize the graph or check for isomorphisms. We just need to check whether the generated graph is `T_k`-free, and record the running minimum theta_ratio (and the graph that generates the minimum theta_ratio). 
    - Note in this step we create a pool of worker threads (controlled by the command line argument). Each worker thread takes one base graph from the queue (obtained from step 3), and add edges to generate graphs on the base. It has it's own instance of EdgeGenerator to do this. And it accumulates the min_theta_ratio value locally, until all graphs are generated on the base. It then push the min_theta_ratio to the global Counters.
    - Two important optimizations explained in the next section.

### EdgeGen optimizations
In this final enumeration phase, it's not computationally feasible to generate all `(K+2)^\binom{n-1}{k-1}-1` graphs for some `K,N` combinations. For example, if `K=4,N=7`, then `(K+2)^\binom{n-1}{k-1} = 6^20 > 3.6* 10^15`, and that's for each base graph. There are almost 30000 base graphs, so the number of graphs generated in this phase would be more than `10^20`. In order to make it computationally feasible, we implement two optimizations.
Note we have not generalized the optimizations for problems other than the main T_k-free problem yet (for example, `forbid_k4` etc. don't use these optimizations).

**Contains T_k optimization**. If `g` together with an edge set contains `T_k` as a subgraph, then `g` together with any superset of that edge set also contains `T_k`. Thus, we skip checking those edge sets. There is no great way to do this in perfect generality, but a simple optimization that is very effective is to skip the "lower -indexed" edges. Take the 6-edge example from the above. Let's say certain direction of edge 2 and edge 3 gives us a graph that contains T_k, where the counter value is `003400`. Then we can, in theory, skip all `xy34zw` for all values of `x,y,z,w`. While this is difficult, we can easily skip `0034zw` for all values of `z,w`. This is implemented in `notify_contain_tk_skip()` in `edge_gen.cpp`.

**Min_theta_ratio optimization**. For a given base graph, we always start by letting `min_theta_ratio` be infinity (`1E8` in code). When we encounter graphs generated from the base graph, with theta_ratio value smaller than the current `min_theta_ratio`, we replace the current `min_theta_ratio` value with that theta_ratio value. Before we generate the next graph, we can cheaply compute its theta_ratio value without copying the edges or doing T_k-free check. If the theta_ratio of the generated graph is >= the current `min_theta_ratio`, we can safely skip it because regardless of the result of the T_k-free check, it will not making the current `min_theta_ratio` smaller. More importantly, in many cases we can skip many candidate graph at once, explained below. There are two related steps in this optimization:
1. Without caring about the direction of each new edge, we can make sure `base_undirected + current_min_theta_ratio * (base_directed + new_edges) > binom_nk`. If this is not true, even if all new edges are directed, the theta_ratio value of the new graph will still be >= current `min_theta_ratio` and therefore uninteresting. In this case we many be able to skip large number of candidate graphs at once. Continue to use the 6-digit counter in this example, suppose the current counter value is `300111`, indicating 4 new edges with `edge[5]` directed and `edge[0],edge[1],edge[2]` undirected. If we find that `base_undirected + current_min_theta_ratio * (base_directed + new_edges) <= binom_nk`, then we will not only skip the current graph from `300111`, but also the next sets `300112, 300113, ..., 300115, 300120, 300121, ..., 300555, 301000, ..., 301110` because none of them will change the inequality for the better and generate an interesting graph. We can jump directly to `301111`, which is the next graph that have more than 4 new edges and therefore may improve the inequality. So in a single step we skipped 6^3 graphs. 
2. The step above assumes all new edges are directed and checks the inequality. If we get past that check, we can further refine the check to account for the directions of the actual edges, to make sure `(base_undirected + new_undirected) + current_min_theta_ratio * (base_directed + new_directed) > binom_nk`. If this is not true, then the current set of edges (with direction) will not give us a graph with theta_ratio value smaller than the current `min_theta_ratio` and therefore we can skip the graph. Similar to the above, we may be able to skip many graphs in one step. Continue using the same example. If the current counter value is `301222` and it doesn't satisfy the inequality, then the next set of graphs `301223, ..., 301225, 301230, ..., 301555, ..., 302000, ..., 302221` will not satisfy the inequality either, because none of them have more directed edges (or same number of directed edges but more undirected edges), to improve the inequality. So we can skip all these and jump to `302222` as the next counter value. 

Intuitively, the Contains T_k optimization works more effectively when the base graph is dense (making the generated graphs more likely to contain T_k), whereas the min_theta_ratio optmization works more effectively when the base graph is sparse (making the generated graph more likely to have a large theta_ratio value). Together, they reduce the computational complexity from 6^20 per base graph to something much more feasible. 

### Graph Data Structure
Everything described in this section is in `graph.h`.

The goal of the Graph data structure is to enable fast operations, especially the following list of operations that must be repeated many times:
* Copying a base graph to a new instance, and add edges in the new instance.
* `T_k`-free check.
* Isomorphism check.

To achieve the goal, the Graph struct contains edges, vertex signatures, and the graph hash, all directly stored in the struct, no pointer indirections.
An edge is stored in a 16-bit struct. 12-bit is used to store the vertex set, by using bit-mask (e.g. binary `001101` means vertex set `{0, 2, 3}`). The other 4-bit is used to store the head of the edge, with `0x0F` meaning undirected, while other value `n` means the head is `n`.
We allow at most 70 edges and embed this edge array directly inside the `Graph` struct, to avoid another heap allocation and a pointer. This means we can run all K values for N<=8, but only some K values for N>8 (the K,N combinations that exceeds the 70-edge limit are not computationally feasible anyway).

The vertex signature and  graph hash are explained in the next section.

### Graph Hashing, Canonicalization, Isomorphism
In the algorithm described above, an important operation is to determine whether two graphs are isomorphic to each other. We'll describe how to make it fast. 

Determining isomorphism is expensive. In order to minimize the number of times we have to run that algorithm, we create a hash for each graph. The property of the hash is that if `G` is isomorphic to `H` then `hash(G)=hash(H)` (but the converse may not be true). To compute the hash:

1. The graph is specified by its edges (described above).
2. From the edges, we can compute the degree information of each vertex, stored in `VertexSignature` struct: undirected degree (number of undirected edges through the vertex), head degree (number of directed edges using the vertex as head), tail degree (number of directed edges through the vertex but not using it as head). After this step, two vertices have the same `VertexSignature` if they have the same degree info. This is implemented in `compute_vertex_signature()`. 
3. Now we can canonicalize the graph: if we permute the vertices, the resulting graph is obviously isomorphic to the original graph. So we permute the vertices in this graph in such a way that the `VertexSignature` array is now sorted in decreasing order according to the degree info. This is sufficient to allow fast determination whether two graphs are isomorphic to each other, if the vertices in each graph are "diverse" or do not look too much alike. This is implemented in `canonicalize()`. Note in `canonicalize()` we also sort the edge array, to make it easier to compare whether two graphs are identical.
4. As part of the canonicalization, we also compute the hash of the entire graph. Since canonicalization already sorted `VertexSignature`s, the array is isomorphism invariant, so we can combine them into the hash, our `graph_hash`. 

We can easily compute whether two graphs are *identical* by simply comparing the edge arrays. This is implemented in `is_identical()`

With the above, we can describe the algorithm to determine isomorphism (implemented in `is_isomorphic()`). 
1. Assume both graphs are canonicalized (debug assert).
2. Simple checks: if hash codes do not match, or edge counts do not match, then false. 
3. If the two graphs are identical, then true.
4. Finally, this is the expensive part: if their `VertexSignature`s match, but they are not identical, we have to perform certain vertex set permutations, and check `is_identical` after each permutation. If `is_identical` is true after any permutation, then true. Otherwise false.
   - We don't need to perform all n! permutations. Instead, we only need to permute within the set of vertices that have the same `VertexSignature`. Because if two vertices have different signatures, we know that they won't produce identical graphs. 
   - We do this by walking through the sorted `VertexSignature` array, find the equal ranges, use the equal ranges to initialize a `Permutator` object (implemented in `permutator.h, cpp`). Then loop through `Permutator.next()` which produces all vertex set permutations that permutes within vertices with the same signatures. 
   
### T_k-free
Subgraph checking for arbitrary partially directed hypergraphs can be quite complicated and expensive (much more complicated than isomorphism check). But luckily, we can check if a k-PDG is `T_k`-free much more quickly. This is implemented in `contains_Tk()` in `forbid_tk/graph_tk.cpp` and relies heavily on bit mask manipulation (as documented further in the code)

## Generalization of the Code & Future Improvements
1. The code at label `v10.1` supports `N<=8` and the code at `v11` supports `N<=12`. The change to support `9<=N<=12` is primarily in the `Edge` struct, where instead of using a `uint8` as the vertex set bit mask, we use 12 bits in a C bit field (and the other 4 bits in the bit field to represent the head). This didn't need any algorithmic changes. The change does mean the compiled binary code is slightly more complex (it needs to do more bit-mask and shifting operations), empirically it's about 1% slower, which is acceptable.
2. The code can be generalized easily to compute other `F`-free scenarios, where `F` is different from T_k described above. The `forbid_k4` directory is an example of this, completely separate from the T_k-free computation code. Also `forbid_k4d0` and `forbid_k4d3` directories contain two other examples of different nature, where the *subgraph defintion is used. 
3. The code provides a pretty fast partially directed hypergraph isomorphism check. We have some deprecated optimizations, such as neighbor hashing ([03d28fd](https://github.com/ThinGarfield/Density-k-PDG/commit/03d28fdc1b032ddf86d3d2060a1bb23202d966c1)) that may further improve runtime in some cases. This may be useful in computations different from T_k-free theta_ratio values. 
4. If the focus is undirected hypergraphs and either performance or space is critical, the data structure and code dealing with `head_vertex` can be removed to make the code more compact and faster (we can also use the entire `uint16` to represent the vertex set bit mask and therefore support `N<=16`). 
