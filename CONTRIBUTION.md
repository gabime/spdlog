# Contributing to spdlog

First of all, thank you for taking the time to contribute to `spdlog`! 
`spdlog` is an extremely fast, header-only/compiled C++ logging library. We welcome contributions of all forms, including bug fixes, new features, and documentation improvements.

This document serves as a guide for contributors to get started easily.

## 1. Getting the Source Code

Start by forking the `gabime/spdlog` repository on GitHub. Then, clone your fork locally:

```bash
git clone https://github.com/<your-username>/spdlog.git
cd spdlog
```

Create a new branch for your feature or bug fix:
```bash
git checkout -b my-new-feature
```

## 2. Building the Project

While `spdlog` can be used as a header-only library, when contributing it is highly recommended to build the compiled library, tests, and examples to ensure your changes are correct and don't break existing functionality.

The project uses CMake. You can configure and build it by running:

```bash
mkdir build
cd build
cmake .. \
  -DSPDLOG_BUILD_TESTS=ON \
  -DSPDLOG_BUILD_EXAMPLE=ON \
  -DSPDLOG_BUILD_WARNINGS=ON
cmake --build . -j
```

## 3. Running Tests

Before submitting a pull request, you must ensure that all unit tests pass. 
`spdlog` uses `ctest` (part of CMake) to run its test suite.

From the `build` directory, run:

```bash
ctest --output-on-failure -j
```

If you are modifying core library code, you should also consider running tests with Address Sanitizer (ASAN) or Thread Sanitizer (TSAN) enabled, which can be done by passing `-DSPDLOG_SANITIZE_ADDRESS=ON` or `-DSPDLOG_SANITIZE_THREAD=ON` to CMake respectively.

## 4. Code Formatting

The `spdlog` codebase uses `clang-format` to enforce a consistent coding style. 

Before committing your changes, please format your code using the provided `.clang-format` file in the root directory. You can format a specific file by running:

```bash
clang-format -i path/to/your/modified_file.h
```

If you use an IDE like VSCode or CLion, you can usually configure it to format your code automatically on save using the `.clang-format` file.

## 5. Submitting a Pull Request

When you are ready to submit your changes:

1. Push your branch to your GitHub fork.
2. Open a Pull Request against the repository.
3. Provide a clear and descriptive title for your PR.
4. Ensure your PR passes all automated tests in the GitHub Actions CI (you'll see the status at the bottom of your PR page).
5. Wait for the maintainers to review your code. They might suggest some changes, which is a normal part of the open-source process!

## Note on Dependencies
`spdlog` bundles the `fmt` library for string formatting by default. Unless you are specifically updating the bundled `fmt` library, you should avoid modifying files in `include/spdlog/fmt/bundled/`.

Thank you for contributing!
