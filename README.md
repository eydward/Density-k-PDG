# Compute Turan Density Coefficient for k-PDG (k-uniform partially directed hypergraphs)

## Preliminary Results (to be verified)

K = number of vertices in each edge

N = number of vertices in each graph

values in the table : minimum theta value across all graphs with N vertices that are T_K free. Where theta(H) is defined as alpha(H) + theta(H) beta(H) =1, alpha(H) is the undirected edge density in the graph, beta(H) is the directed edge density in the graph.

| N   | K=2 | K=3   | K=4   | K=5   | K=6   | K=7   |
| --- | --- | ----- | ----- | ----- | ----- | ----- |
| 2   |   1 |       |       |       |       |       |
| 3   | 3/2 |   1   |       |       |       |       |
| 4   | 3/2 | 4/3   |     1 |       |       |       |
| 5   | 5/3 | 5/3   |   5/4 |     1 |       |       |
| 6   | 5/3 | 5/3   |   3/2 |   6/5 |     1 |       |
| 7   | 7/4 | ?     | ?     |   7/5 |   7/6 |     1 |
| 8   | 7/4 |  ?    | ?     |   ?   |   ?   |   8/7 |


## Setup
* In order to build and run the code, c++ 20 compatible compiler is required. My environment uses `gcc (Rev8, Built by MSYS2 project) 11.2.0`, but any recent release of gcc should work. 
* It also requires bazel (see https://bazel.build/install). My global bazel config is the following (will be different if you use Linux instead of Windows)
```
startup --host_jvm_args=--add-opens=java.base/java.nio=ALL-UNNAMED --host_jvm_args=--add-opens=java.base/java.lang=ALL-UNNAMED
build --compiler=mingw-gcc --action_env=BAZEL_CXXOPTS="-std=c++20"
test --test_output=errors
```

## Usage
Run all commands in the project root directory. 
* To execute all unit tests: `bazel test ...`
* To run the program in DEBUG mode (slow, for debugging only): `bazel run -c dbg src:kPDG <K> <N>` 
* To run the program in OPTIMIZED mode (fast): `bazel run -c opt src:kPDG <K> <N>`
* Alternatively, run `bazel build -c opt ...` and then find the executable in `bazel-out\src\` and execute it manually with `kPDG <K> <N>`.

Note the code currently is single-threaded, doesn't take advantage of multi-core machines and certainly not capable to run in distributed environment. This is TODO. 

## Organization of Code
All source code are in the `src` directory.
- `BUILD`: the project that instructs `bazel` how to build, run, and test.
- `tests/*cpp` : unit tests, not part of actual execution.
- `kPDG.cpp`: entry point of the command line program.
- `graph.h, .cpp`: declaration and implementation of the Graph struct, as well as the definition of `Edge` and `VertexSignature`. This is where isomorphism check, hashing, canonicalization, and T_k-free checks are implemented.
- `grower.h, .cpp`: declaration and implementation of growing the search tree, see algorithm design below. 
- `permutator.h, .cpp`: utility function to generate all permutations with specified ranges.
- `fraction.h, .cpp`: simple implementation of a fraction. (We store the theta value as a fraction).
- `counters.h, .cpp`: the header and implementation of a bunch of statistical counters. The minimum theta value is stored here with the graph producing it. Also a bunch of data used to track the performance of the algorithm.
- `allocator.h, .cpp`: a simple memory allocator to create Graph objects. We used to store the Graph object inline inside `std::unordered_set` but when it grows hundreds of millions of entries and many GB of data, the performance becomes really bad, so we use this allocator and only put pointers in `std::unordered_set` in the Grower implementation.
- `edge_gen.h, cpp`: utility to generate edge sets to be added to an existing graph, in order to grow the search tree. 

## Algorithm Design Summary
### Growing the Search Tree
We use a simple idea to grow the search tree while try to avoid unnecessary work where we can. This is implemented in `grower.h, .cpp` with the edge set enumeration logic implemented in `edge_gen.h, .cpp`.  Note all graphs are on N vertices {0,1,...,N-1}. When we say "a graph with n vertices" in this section, we mean the number of vertices that have positive degrees.

1. Start with K-1 vertices {0,...,K-2} and 0 edge. This is the only graph in `canonicals[K-1]`.
2. Let n = K.  Let `canonicals[n]` be empty set. 
4. Repeat until n > N:
    - Add vertex (n-1) to the vertex set.
    - General all edge candidates that goes through the new vertex, by finding all \binom{n-1}{k-1} vertex sets without vertex (n-1), and adding vertex (n-1) to each edge. (This is in `initialize()` in `edge_gen.cpp`.)
    - For each graph g in the canonical set, we want to add all possible combinations of the edge candidates (`(K+2)^\binom{n-1}{k-1}-1` in total) to g and see if we can generate additional graphs that are not isomorphic to anything already in the canonical set. This is done in `next()` in `edge_gen.cpp`. Basically run a `\binom{n-1}{k-1}` digit counter, the value of each digit goes from 0 to K+1. 0 means the corresponding edge is not in the set to be added to g. 1 means the corresponding edge is in the set to be added to g, as an undirected edge. >=2 means the corresponding edge is in the set to be added to g, as a directed edge, with the value indicating the head. For example if the edge candidates has 6 edges, then we run a 6-digit counter: `000001`, `000002`, ... , `00000(K+1)`, `000010`, `000011`, ... `(K+1)(K+1)(K+1)(K+1)(K+1)(K+1)`. And each counter value corresponds to a set of edges to be added to g.
        * For each edge set: make a copy of g, add the edge set to the copy, check whether it's T_k-free (explained below). 
        * If it contains T_k as a subgraph, ignore it and move on to the next edge set from the edge generator.
        * If it's T_k-free, canonicalize it (explained below), and check if it is isomorphic to some graph already stored in `canonicals[n]`. If it's not isomorphic to any existing graph, add it to `canonicals[n]`. Here we also check if the new graph gives us a lower theta value than previously observed, if so remembers the theta value, and the graph that produced it. 
    - As an important optimization, if g together with an edge set contains T_k as a subgraph, then g together with any superset of that edge set also contains T_k. So we should skip checking those edge sets. There is no great way to do this in the most general way, but a simple optimization that is reasonably effective is to skip the "lower" edges. Take the 6-edge example from the above. Let's say certain direction of edge 2 and edge 3 gives us a graph that contains T_k, where the counter value is `003400`. Then in theory we can skip all `xy34zw` for all values of `x,y,z,w`. That's hard to do, but we can easily skip all `0034zw`. This is implemented in `notify_contain_tk_skip()` in `edge_gen.cpp`.

### Graph Data Structure
The goal is to store the graph using least number of bytes possible, for two reasons: (1) I run out of memory even on a desktop computer with 64GB memory when computing for `K=4,N=7`. (2) Smaller the data structure means better cache efficiency and faster execution time. Everthing described in this section is in `graph.h`. An edge is stored in a 16-bit struct. 8-bit is used to store the vertex set, by using bit-mask (e.g. binary `001101` means vertex set `{0, 2, 3}`). The other 8-bit is used to store the head of the edge. `0xFF` means undirected, while other value n means the head is n.

We allow at most 35 edges and embed this edge array directly inside the `Graph` struct, to avoid another heap allocation and a pointer. This means we can run all K value for N<=7, but only some K values for N=8.

The Graph struct also contains a few piece of info, including `graph_hash` (explained below). For the total of 80 bytes. By assuming N<=7 we can make it even smaller (vertex set plus head can occupy 10 bytes, which would make the Graph struct 56 bytes). I may need this optimization if I can't fit the compute on a large RAM computer. 

### Graph Hashing, Canonicalization, Isomorphism
In the algorithm described above, an important operation is to determine whether two graphs are isomorphic to each other. We'll describe how to make it fast. I found no references describing algorithms to determine partially directed hypergraph isomorphism (or even hypergraph isomorphism, except to use line graph to reduced it to graph isomorphism problem). I did find  two papers from McKay [Practical Graph Isomorphism](http://users.cecs.anu.edu.au/~bdm/papers/pgi.pdf) and [Practical Graph Isomorphism, II](https://arxiv.org/abs/1301.1493). I borrowed some ideas (looking at neighbors when characterizing a vertex), but didn't use the algorithm described in the paper (generalizing to hypergraph didn't seem to be practical except for using line graphs, which would lead to pretty large bipatite graphs and I think it will be slower than my implementation). Here is what I implemented in code. 

Determining isomorphism is expensive. In order to minimize the number of times we have to run that algorithm, we create a hash for each graph. The property of the hash is that if G is isomorphism to H then hash(G)=hash(H) (but the converse may not be true). To compute the hash:

1. The graph is specified by it
