# Changelog

## v2.0.0 (Unreleased)

### Breaking Changes

* **C++17 minimum required** - Raised from C++11 in v1.x.
* **Header-only mode removed** - spdlog is now a compiled library only. `SPDLOG_HEADER_ONLY` and `SPDLOG_COMPILED_LIB` options are gone. All `-inl.h` files are removed.
* **CMake minimum version raised to 3.23**.
* **`filename_t` is now `std::filesystem::path`** instead of `std::string`.
* **Log level type changed** - `level::level_enum` is now `enum class level : uint8_t`.
* **Logger creation simplified** - convenience functions like `spdlog::basic_logger_mt(...)` are replaced with `spdlog::create<SinkType>(name, args...)`.
* **Async API changed** - `spdlog/async.h`, `spdlog::async_logger`, and `spdlog::init_thread_pool()` are removed. Use `spdlog/sinks/async_sink.h` with the new `async_sink` class instead. See README for examples.
* **Global registry removed** - `spdlog::get()`, `spdlog::register_logger()`, `spdlog::drop()`, `spdlog::drop_all()`, `spdlog::apply_all()` are all removed.
* **Default logger renamed** - `spdlog::default_logger()` is now `spdlog::global_logger()`. Similarly `set_default_logger()` is now `set_global_logger()`.
* **`std::format` support removed** - The `SPDLOG_USE_STD_FORMAT` option is gone. spdlog now uses [fmt](https://github.com/fmtlib/fmt) exclusively (bundled 12.1.0 by default, or external via `SPDLOG_FMT_EXTERNAL=ON`).
* **Bundled fmt headers moved** - `spdlog/fmt/bundled/` is gone. fmt is now fetched via CMake. Include `fmt/` headers directly.
* **Configuration module removed** - `spdlog/cfg/env.h` and `spdlog/cfg/argv.h` are gone. `spdlog::cfg::load_env_levels()` and `spdlog::cfg::load_argv_levels()` no longer exist.
* **Backtrace feature removed** - `spdlog::enable_backtrace()` and `spdlog::dump_backtrace()` are gone.
* **Wide-char support removed** - `SPDLOG_WCHAR_TO_UTF8_SUPPORT` and `SPDLOG_WCHAR_FILENAMES` options are gone.
* **`spdlog::flush_every()` removed**.
* **`SPDLOG_EOL` define removed**.
* **`tweakme.h` removed** - compile-time options are now in CMakeLists.txt.
* **Logger destructor is no longer virtual** - `~logger() = default`.
* **`log_msg.level` field renamed to `log_msg.log_level`** - affects custom sink implementations.
* **All log methods are now `noexcept`**.
* **`SPDLOG_NOEXCEPT` and `SPDLOG_CONSTEXPR` macros removed** - replaced with standard C++17 keywords.

### New Features

* New `async_sink` class with configurable overflow policies (`block`, `overrun_oldest`, `discard_new`).
* `async_sink::with<Sink>(...)` convenience factory for creating async sinks.
* Thread start/stop callbacks in async sink configuration.
* `source_loc` now includes `short_filename` (auto-computed basename).
* Centralized error handling via `err_helper` with rate limiting.
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
