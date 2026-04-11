# Changelog

## v2.0.0 (Unreleased)

### Breaking Changes

* **C++17 minimum required** - Raised from C++11 in v1.x.
* **Header-only mode removed** - spdlog is now a compiled library only. `SPDLOG_HEADER_ONLY` and `SPDLOG_COMPILED_LIB` options are gone.
* **`std::format` support removed** - The `SPDLOG_USE_STD_FORMAT` option is gone. spdlog now uses [fmt](https://github.com/fmtlib/fmt) exclusively (bundled 12.1.0 by default, or external via `SPDLOG_FMT_EXTERNAL=ON`).
* **Configuration module removed** - `spdlog/cfg/env.h` and `spdlog/cfg/argv.h` are gone. `spdlog::cfg::load_env_levels()` and `spdlog::cfg::load_argv_levels()` no longer exist.
* **Backtrace feature removed** - `spdlog::enable_backtrace()` and `spdlog::dump_backtrace()` are gone.
* **Async API changed** - `spdlog/async.h` is removed. Use `spdlog/sinks/async_sink.h` with the new `async_sink` class instead. See README for examples.
* **Wide-char support removed** - `SPDLOG_WCHAR_TO_UTF8_SUPPORT` and `SPDLOG_WCHAR_FILENAMES` options are gone.
* **`SPDLOG_EOL` define removed**.
* **Logger destructor is no longer virtual** - `~logger() = default`.
* **CMake minimum version raised to 3.23**.

### New Features

* New `async_sink` class with configurable overflow policies (`block`, `overrun_oldest`, `discard_new`).
* `async_sink::with<Sink>(...)` convenience factory for creating async sinks.
* Thread start/stop callbacks in async sink configuration.
* Improved `constexpr` and `noexcept` usage throughout.

### Migrating from v1.x

#### Async logging

Before (v1.x):
```c++
#include "spdlog/async.h"
spdlog::init_thread_pool(8192, 1);
auto logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_logger", "logs/async.txt");
```

After (v2.x):
```c++
#include "spdlog/sinks/async_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
auto sink = spdlog::sinks::async_sink::with<spdlog::sinks::basic_file_sink_mt>("logs/async.txt");
auto logger = std::make_shared<spdlog::logger>("async_logger", sink);
```
