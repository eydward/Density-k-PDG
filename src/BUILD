load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name="graph",
    hdrs=[
        "allocator.h",
        "counters.h",
        "edge_gen.h",
        "fraction.h",
        "graph.h",
        "grower.h",
        "permutator.h",
    ],
    srcs=[
        "allocator.cpp",
        "counters.cpp",
        "edge_gen.cpp",
        "fraction.cpp",
        "graph.cpp",
        "grower.cpp",
        "permutator.cpp",
    ],
    copts=["-std=c++20"],
)

cc_binary(
    name="kPDG",
    srcs=["kPDG.cpp"],
    deps=[":graph"],
    copts=["-std=c++20"],
    linkstatic=True,
    features = [
        "fully_static_link",
    ],
)

cc_test(
    name="graph_test",
    srcs=[
        "tests/edge_gen_test.cpp",
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
