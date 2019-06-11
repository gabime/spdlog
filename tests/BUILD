cc_test(
  name="main",
  srcs=glob([
    "*.h*",
    "utils.cpp",
    "main.cpp",
    "test_*.cpp"
    ],
    exclude=[
      "test_systemd.cpp"
    ]
  ) + select({
    "@bazel_tools//src/conditions:windows": [],
    "@bazel_tools//src/conditions:darwin": [],
    "//conditions:default": ["test_systemd.cpp"],
  }),
  includes=[
    "tests"
  ],
  deps=[
    "//:headers"
  ]
)
