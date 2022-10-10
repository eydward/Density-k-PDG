# Development

*This document outlines the build/test environment, how to modify the code, and how to contribute to the project.
As a prerequisite, readers should be familiar with the content in 
[Math, Data Structure, and Algorithms](./math_alg.md).*

## Development Environment Setup

In order to build and run the code, a C++ 17 compatible compiler is required. I use `gcc 11.2.0`, but any `gcc-11` release  should work. 
  - To install `gcc` on Windows, follow instructions in this 
    [section](https://code.visualstudio.com/docs/languages/cpp#_example-install-mingwx64).
  - On Debian-based Linux distros (including Ubuntu) just run 
    `sudo apt install build-essential`. This is usually sufficient, but in case
    some older Ubuntu versions don't have `gcc-11` in the default repo, follow instructions 
    [here](https://stackoverflow.com/questions/67298443/when-gcc-11-will-appear-in-ubuntu-repositories)
    to install `gcc-11`. On Linux distros that don't use `apt`, Google search the installation steps.

This project also requires `bazel`; follow instructions on https://bazel.build/install to install. 
On Windows the recommended global `bazel` config is the following (no config is required on Linux):
  ```
  startup --host_jvm_args=--add-opens=java.base/java.nio=ALL-UNNAMED --host_jvm_args=--add-opens=java.base/java.lang=ALL-UNNAMED
  build --compiler=mingw-gcc --action_env=BAZEL_CXXOPTS="-std=c++20"
  test --test_output=errors
  ```

Once these have been installed and properly configured, you can proceeed as follows. Run all commands in the
project root directory. The command line arguments are explained in
[Math, Data Structure, and Algorithms](./math_alg.md).
* To execute all unit tests: `bazel test ...`
* To run the program in DEBUG mode (slow, for debugging only): `bazel run -c dbg src:kPDG <K> <N> <T>` 
* To run the program in OPTIMIZED mode (fast): `bazel run -c opt src:kPDG <K> <N> <T>`
* Alternatively, run `bazel build -c opt ...` and then find the executable in `bazel-out\src\` and execute it manually with `kPDG <K> <N> <T>`.
* To get line level test coverage, first run `bazel coverage ...`, 
then `genhtml -o bazel-testlogs/<dir> bazel-testlogs/<dir>/coverage.dat`. It generates a html file
in the `bazel-testlogs/<dir>` directory, with test coverage summary, plus links to each source
code file with line by line coverage info.
 
We cannot use static linking on Linux. On Windows, it's not necessary to use static linking if you use the same computer to build and run. But if you plan to use one computer to build, and another to run, then the most convenient way is to use static linking. Use this command `bazel build -c opt --features fully_static_link ...`.

## Organization of Code
All source code is in the `src` directory.
- `BUILD`: the project that instructs `bazel` how to build, run, and test.
- `tests/*cpp` : unit tests and stress tests. Not part of the actual program.
- `collector/*` : a utility to collect the data from log files of multiple batches, validate consistency, and summarize the final result. This is used to get the result for `K=4,N=7`. Not part of the main program.
- `forbid_k4/*` : to solve a different problem (see header file in the directory for details). Not part of the main program.
- `forbid_k4d0/*`, `forbid_k4d3/*` : to solve two different problems (see header file in the directory for details). Not part of the main program.
- `kPDG.cpp`: entry point of the main program.
- `graph.h, .cpp`: declaration and implementation of the Graph struct, as well as the definition of `Edge` and `VertexSignature`. This is where isomorphism check, hashing, and canonicalization are implemented.
- `grower.h, .cpp`: declaration and implementation of growing the search tree, see algorithm design below. 
- `forbid_tk/graph_tk.h, .cpp`: implements the $T_k$-free check. 
- `permutator.h, .cpp`: simple utility function to generate all permutations with specified ranges.
- `fraction.h, .cpp`: simple implementation of a fraction. (We store the theta_ratio value as a fraction).
- `counters.h, .cpp`: the header and implementation of a bunch of statistical counters. The minimum theta_ratio value is stored here with the graph producing it. Also produces data used to track the performance of the algorithm.
- `edge_gen.h, cpp`: utility to generate edge sets to be added to an existing graph, in order to grow the search tree. 

## Contributing to the Project
The code is open-source with a permissive MIT [license](../LICENSE). Feel free to fork and modify, to use in your
own work. If the modifications may benefit others, please consider contributing back by submitting
a pull request. 

Pull request requirements:

1. All assumptions and optimizations must be mathematically correct. If your code rely on certain
theorems, please include a link to the proofs in the comment.
2. All code must have 100% test coverage (see instructions above).
3. All code must be well documented, with detailed comments explaining the motivations and behaviors.
3. Please follow the same coding style (variable/function naming, indentation and spacing, 
   bracket placement, etc.) as the existing code. I use the 
   [Google coding style](https://google.github.io/styleguide/cppguide.html), with column limit 100.

