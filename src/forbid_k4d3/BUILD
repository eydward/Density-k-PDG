load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
  name="graph_forbid_k4d3",
  hdrs=["graph_k4d3.h"],
  srcs=["graph_k4d3.cpp"],
  deps=["//src:graph"],
  copts=["-std=c++17"],
)

# cc_binary(
#     name="kPDG-forbid-k4d3",
#     srcs=["kpdg-forbid-k4d3.cpp"],
#     deps=[":graph_forbid_k4d3"],
#     copts=["-std=c++17"],
#     linkopts=["-lpthread"],
#     linkstatic=True,
# )

cc_test(
    name="graph_forbid_k4d3_test",
    srcs=["tests/graph_k4d3_test.cpp"],
    deps=[
        ":graph_forbid_k4d3",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    copts=["-std=c++17"],
    linkopts=["-lpthread"],
)
