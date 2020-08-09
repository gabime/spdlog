# How to use spdlog with Bazel?

1. Add to your `WORKSPACE` file this:

    ```python
    http_archive(
        name = "spdlog",
        build_file = "//:spdlog.BUILD",
        sha256 = "f0114a4d3c88be9e696762f37a7c379619443ce9d668546c61b21d41affe5b62",
        strip_prefix = "spdlog-1.7.0",
        urls = ["https://github.com/gabime/spdlog/archive/v1.7.0.tar.gz"],
    )
    ```

    

2. Copy `spdlog.BUILD` to your workspace directry.

Note: If you make use of the FMT library already in your project you can use that one also for spdlog: Remove the comments in the `spdlog.BUILD` file.

**Example**

This directory contains `WORKSPACE.bazel` file that how to integrate `spdlog` into your Bazel project.

To test it run:

```bash
bazel run //:HelloWorld
```

