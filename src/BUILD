load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "graph",
  hdrs = ["graph.h", "graph.hpp", "grower.h", "permutator.h"],
  srcs = [],
  copts = ["-std=c++20"],
)

cc_binary(
  name = "kPDG",
  srcs = ["kPDG.cpp"],
  deps = [":graph"],
  copts = ["-std=c++20"],
)

cc_test(
  name = "graph_test",
  srcs = ["tests.cpp"],
  deps = [
    ":graph",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
  copts = ["-std=c++20"],
)