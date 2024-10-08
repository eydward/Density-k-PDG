load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_library(
    name="graph",
    hdrs=[
        "counters.h",
        "edge_gen.h",
        "fraction.h",
        "graph.h",
        "grower.h",
        "permutator.h",
        "forbid_tk/graph_tk.h",
    ],
    srcs=[
        "counters.cpp",
        "edge_gen.cpp",
        "fraction.cpp",
        "graph.cpp",
        "forbid_tk/graph_tk.cpp",
        "grower.cpp",
        "permutator.cpp",
    ],
    copts=["-std=c++17"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name="kPDG",
    srcs=["kPDG.cpp"],
    deps=[":graph"],
    copts=["-std=c++17"],
    linkopts=["-lpthread"],
    linkstatic=True,
)


cc_binary(
    name="collector",
    srcs=["collector/collector.cpp"],
    deps=[":graph"],
    copts=["-std=c++17"],
    linkstatic=True,
)

cc_library(
    name="iso_stress_test",
    hdrs=["tests/iso_stress_test.h"],
    srcs=["tests/iso_stress_test.cpp"],
    deps=[":graph"],
    copts=["-std=c++17"],
)

cc_binary(
    name="isostress",
    srcs=["tests/isostress.cpp"],
    deps=[":iso_stress_test"],
    copts=["-std=c++17"],
    linkstatic=True,
)

cc_binary(
    name="edgegenstress",
    srcs=["tests/edgegenstress.cpp"],
    deps=[":graph"],
    copts=["-std=c++17"],
    linkopts=["-lpthread"],
    linkstatic=True,
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
        ":iso_stress_test",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
    copts=["-std=c++17"],
    linkopts=["-lpthread"],
)
