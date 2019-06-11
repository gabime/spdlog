cc_library(
    name = "headers",
    visibility = ["//visibility:public"],
    hdrs = glob(["include/**/*.h*"]),
    includes = ["include"],
)


cc_library(
    name = "spdlog",
    visibility = ["//visibility:public"],
    defines=[
      "SPDLOG_COMPILED_LIB"
    ],
    srcs = ["src/spdlog.cpp"],
    includes = ["include"],
)



