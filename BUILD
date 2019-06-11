cc_library(
    name = "headers",
    visibility = ["//visibility:public"],
    hdrs = glob(["include/**/*.h*"]),
    includes = ["include"],
)

cc_binary(
  name="bench",
  srcs=[
    "bench/utils.h",
    "bench/bench.cpp"
  ],
  includes=[
    "bench"
  ],
  deps=[
    ":headers"
  ]
)

cc_binary(
  name="async_bench",
  srcs=[
    "bench/utils.h",
    "bench/async_bench.cpp"
  ],
  includes=[
    "bench"
  ],
  deps=[
    ":headers"
  ]
)

cc_binary(
  name="latency",
  srcs=[
    "bench/utils.h",
    "bench/latency.cpp"
  ],
  includes=[
    "bench"
  ],
  deps=[
    ":headers"
  ]
)

cc_binary(
  name="example",
  srcs=[
    "example/example.cpp"
  ],
  includes=[
    "bench"
  ],
  deps=[
    ":headers"
  ]
)