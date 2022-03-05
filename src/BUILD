load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name="graph",
    hdrs=[
        "counters.h",
        "edge_gen.h",
        "fraction.h",
        "graph.h",
        "graph.hpp",
        "grower.h",
        "grower.hpp",
        "permutator.h",
    ],
    srcs=[
        "counters.cpp",
        "fraction.cpp",
    ],
    copts=["-std=c++20"],
)

cc_binary(
    name="kPDG",
    srcs=["kPDG.cpp"],
    deps=[":graph"],
    copts=["-std=c++20"],
)

cc_test(
    name="graph_test",
    srcs=[
        "tests/fraction_test.cpp",
        "tests/grower_test.cpp",
        "tests/permutator_test.cpp",
        "tests/graph_test.cpp",
    ],
    deps=[
        ":graph",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
    copts=["-std=c++20"],
)
