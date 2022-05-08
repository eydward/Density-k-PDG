# Running Tests

At project root directory, run `bazel test ...`

# Test Coverage

When making changes, make sure the code has 100% test coverage. To get coverage info, run the 
following at the project root directory:
   ```
   bazel coverage ...
   genhtml -o bazel-testlogs/src/ bazel-testlogs/src/graph_test/coverage.dat bazel-testlogs/src/forbid_k4/graph_forbid_k4_test/coverage.dat bazel-testlogs/src/forbid_k4d0/graph_forbid_k4d0_test/coverage.dat bazel-testlogs/src/forbid_k4d3/graph_forbid_k4d3_test/coverage.dat
   ```
The above commands create `bazel-testlogs/src/index.html`, load in browser and you can see
line coverage info. 