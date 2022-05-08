load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name="graph_forbid_k4",
  hdrs=["graph_k4.h"],
  srcs=["graph_k4.cpp"],
  deps=["//src:graph"],
  copts=["-std=c++17"],
)

cc_binary(
    name="kPDG-forbid-k4",
    srcs=["kpdg-forbid-k4.cpp"],
    deps=[":graph_forbid_k4"],
    copts=["-std=c++17"],
    linkopts=["-lpthread"],
    linkstatic=True,
)

cc_test(
    name="graph_forbid_k4_test",
    srcs=["graph_k4_test.cpp"],
    deps=[
        ":graph_forbid_k4",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    copts=["-std=c++17"],
    linkopts=["-lpthread"],
)
