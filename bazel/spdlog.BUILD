# Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
# Distributed under the MIT License (http://opensource.org/licenses/MIT)

cc_library(
    name = "spdlog",
    srcs = [
        "src/async.cpp",
        "src/cfg.cpp",
        "src/color_sinks.cpp",
        "src/file_sinks.cpp",
        "src/fmt.cpp",
        "src/spdlog.cpp",
        "src/stdout_sinks.cpp",
    ],
    hdrs = glob(["include/**/*.h*"]),
    defines = [
        "SPDLOG_COMPILED_LIB",
        #"SPDLOG_FMT_EXTERNAL",
    ],
    includes = ["include"],
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [],
        "@bazel_tools//src/conditions:darwin": [],
        "//conditions:default": ["-lpthread"],
    }),
    visibility = ["//visibility:public"],
    #deps = ["@fmt"],
)
