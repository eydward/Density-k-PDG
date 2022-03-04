# Compute Turan Density Coefficient for k-PDG (k-uniform partially directed hypergraphs)

## Preliminary Results (to be verified)

k = number of vertices in each edge

n = number of vertices in each graph

values in the table : minimum theta value across all graphs with n vertices that are T_k free. Where theta(H) is defined as alpha(H) + theta(H) beta(H) =1, alpha(H) is the undirected edge density in the graph, beta(H) is the directed edge density in the graph.

| n   | k=2 | k=3   | k=4   | k=5   | k=6   |
| --- | --- | ----- | ----- | ----- | ----- |
| 2   |   1 |       |       |       |       |
| 3   | 3/2 |   1   |       |       |       |
| 4   | 3/2 | 3/2   |     1 |       |       |
| 5   | 5/3 | 9/5   |   3/2 |     1 |       |
| 6   | 5/3 | 19/10 |  13/7 |   3/2 |     1 |
| 7   | 7/4 | ?     | ?     |   9/5 |   3/2 | 
| 8   |   ? |  ?    | ?     |   ?   |   ?   |

**Note: the k=5, n=7, min_theta=9/5 should give original result, and prove the k-SAT counting conjecture by Bollobás, Brightwell, and Leader, for k=5 and k=6, following the "ENUMERATING k-SAT FUNCTIONS" paper.**

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

## Algorithm Design Summary
TODO
