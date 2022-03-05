# Compute Turan Density Coefficient for k-PDG (k-uniform partially directed hypergraphs)

## Preliminary Results (to be verified)

K = number of vertices in each edge

N = number of vertices in each graph

values in the table : minimum theta value across all graphs with N vertices that are T_K free. Where theta(H) is defined as alpha(H) + theta(H) beta(H) =1, alpha(H) is the undirected edge density in the graph, beta(H) is the directed edge density in the graph.

| N   | K=2 | K=3   | K=4   | K=5   | K=6   |
| --- | --- | ----- | ----- | ----- | ----- |
| 2   |   1 |       |       |       |       |
| 3   | 3/2 |   1   |       |       |       |
| 4   | 3/2 | 3/2   |     1 |       |       |
| 5   | 5/3 | 9/5   |   3/2 |     1 |       |
| 6   | 5/3 | 19/10 |  13/7 |   3/2 |     1 |
| 7   | 7/4 | ?     | ?     |   9/5 |   3/2 | 
| 8   |   ? |  ?    | ?     |   ?   |   ?   |

**Note: K=5, N=7, min_theta=9/5  should give original result, and prove the k-SAT counting conjecture by Bollobás, Brightwell, and Leader, for k=5 and k=6, following the "ENUMERATING k-SAT FUNCTIONS" paper.**

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
* To run the program in DEBUG mode (slow, for debugging only): `bazel run -c dbg src:kPDG`
* To run the program in OPTIMIZED mode (fast): `bazel run -c opt src:kPDG`
* Alternatively, run `bazel build -c opt ...` and then find the executable in `bazel-out\src\` and execute it manually.

Note in order to optimize for speed, we try to avoid dynamic memory allocation as much as possible, so it uses C++ template and fixed arrays. Unfortunately it makes it a bit less friendly. To change configuration (k and n), you have to edit the kPDG.cpp file in the source directory, and re-compile. There is no command-line arguments for this purpose.

Note the code currently is single-threaded, doesn't take advantage of multi-core machines and certainly not capable to run in distributed environment. This is TODO. 

## Organization of Code
All source code are in the `src` directory.
- `BUILD`: the project that instructs `bazel` how to build, run, and test.
- `tests.cpp` : unit tests, not part of actual execution.
- `kPDG.cpp`: entry point. Edit this file to put in the appropriate K and N values before compile and run.
- `graph.h`: summary declarations of the Graph struct, as well as the definition of `Edge` and `VertexSignature`.
- `graph.hpp`: the detailed implementation of the Graph struct. Because we use C++ template, the compiler requires the implementation to be in a header file to include, instead of in a cpp file to link. 
- `grower.h`: the implementation of growing the search tree, see algorithm design below. 
- `permutator.h`: utility function to generate all permutations with specified ranges.
- `fraction.h` and `fraction.cpp`: the header and implementation of a fraction. (We store the theta value as a fraction).
- `counters.h` and `counters.cpp`: the header and implementation of a bunch of counters. The minimum theta value is stored here with the graph producing it. Also contains a bunch of other statistical counters used to track the performance of the algorithm.

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
