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
We use a simple idea to grow the search tree while try to avoid unnecessary work. This is implemented in `grower.h`.  Note all graphs are on N vertices {0,1,...,N-1}. When we say "a graph with n vertices" in this section, we mean the number of vertices that appear in some edges is n.

1. Start with K-1 vertices {0,...,K-2} and 0 edge. 
2. Let n = K.  Let canonicals be empty set. 
4. Repeat until n > N:
    - Add vertex (n-1) to the vertex set.
    - General all edge candidates that goes through the new vertex, by finding \binom{n-1}{k-1} sets without vertex (n-1), and add vertex (n-1) to each edge.
    - For each graph g in the canonical set, we want to add all possible combinations of the edge candidates (2^\binom{n-1}{k-1} in total) to g and see if we can generate additional graphs that are not isomorphic to anything already in the canonical set. 
        *  
