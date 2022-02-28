load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name = "graph",
  hdrs = ["graph.h"],
  srcs = [],
)

cc_binary(
  name = "kPDG",
  srcs = ["kPDG.cpp"],
  deps = [":graph"],
)

cc_test(
  name = "graph_test",
  srcs = ["tests.cpp"],
  deps = [
    ":graph",
    "@gtest//:gtest",
    "@gtest//:gtest_main",
  ],
)