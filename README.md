# spdlog


[![ci](https://github.com/gabime/spdlog/actions/workflows/linux.yml/badge.svg)](https://github.com/gabime/spdlog/actions/workflows/linux.yml)&nbsp;
[![ci](https://github.com/gabime/spdlog/actions/workflows/windows.yml/badge.svg)](https://github.com/gabime/spdlog/actions/workflows/windows.yml)&nbsp;
[![ci](https://github.com/gabime/spdlog/actions/workflows/macos.yml/badge.svg)](https://github.com/gabime/spdlog/actions/workflows/macos.yml)&nbsp;
[![Release](https://img.shields.io/github/release/gabime/spdlog.svg)](https://github.com/gabime/spdlog/releases/latest)

Fast C++ logging library

## Install
```console
$ git clone https://github.com/gabime/spdlog.git
$ cd spdlog && mkdir build && cd build
$ cmake .. && cmake --build .
```

see example [CMakeLists.txt](https://github.com/gabime/spdlog/blob/v2.x/example/CMakeLists.txt) on how to use.

## Platforms
* Linux, FreeBSD, OpenBSD, Solaris, AIX
* Windows (msvc, cygwin)
* macOS 
* Android

## Package managers:
* Debian: `sudo apt install libspdlog-dev`
* Homebrew: `brew install spdlog`
* MacPorts: `sudo port install spdlog`
* FreeBSD:  `pkg install spdlog`
* Fedora: `dnf install spdlog`
* Gentoo: `emerge dev-libs/spdlog`
* Arch Linux: `pacman -S spdlog`
* openSUSE: `sudo zypper in spdlog-devel`
* vcpkg: `vcpkg install spdlog`
* conan: `conan install --requires=spdlog/[*]`
* conda: `conda install -c conda-forge spdlog`
* build2: ```depends: spdlog ^2.0.0```


## What's new in v2.x

spdlog v2.x is a major rewrite with cleaner design and a simpler API. Key changes from v1.x:

* **C++17 required** (v1.x supported C++11). Uses `std::filesystem`, `std::string_view`, etc.
* **Compiled library only** - header-only mode has been removed. All `-inl.h` files are gone.
* **CMake minimum version raised to 3.23**.
* **`filename_t` is now `std::filesystem::path`** instead of `std::string`.
* **Log level type changed** - `level::level_enum` is now `enum class level : uint8_t`.
* **Simpler logger creation** - convenience functions like `spdlog::basic_logger_mt(...)` are replaced with a single template: `spdlog::create<SinkType>(logger_name, sink_args...)`.
* **New async model** - the global thread pool (`spdlog::init_thread_pool()`) and `spdlog::async_logger` are replaced by `async_sink`, a regular sink with its own worker thread and configurable overflow policy (`block`, `overrun_oldest`, `discard_new`).
* **No global registry** - `spdlog::get()`, `spdlog::register_logger()`, `spdlog::drop()`, `spdlog::drop_all()`, `spdlog::apply_all()` are all removed. Use `spdlog::global_logger()` to access the default logger, or hold your own `shared_ptr<logger>`.
* **Default logger renamed** - `spdlog::default_logger()` is now `spdlog::global_logger()`.
* **`std::format` support removed** - `SPDLOG_USE_STD_FORMAT` is gone. Uses [fmt](https://github.com/fmtlib/fmt) exclusively.
* **Bundled fmt headers moved** - `spdlog/fmt/bundled/` is gone. fmt is now fetched via CMake. Include `fmt/` headers directly.
* **Backtrace feature removed** - `spdlog::enable_backtrace()` and `spdlog::dump_backtrace()` are gone.
* **Configuration module removed** - `spdlog/cfg/env.h` and `spdlog/cfg/argv.h` (env/argv level loading) are gone.
* **Wide-char support removed** - `SPDLOG_WCHAR_TO_UTF8_SUPPORT` and `SPDLOG_WCHAR_FILENAMES` are gone.
* **`spdlog::flush_every()` removed**.
* **`SPDLOG_EOL` define removed**.
* **`tweakme.h` removed** - compile-time options are now in CMakeLists.txt.
* **Logger destructor is no longer virtual**.
* **`log_msg.level` field renamed to `log_msg.log_level`** (affects custom sinks).
* **All log methods are now `noexcept`**.

See [CHANGELOG.md](CHANGELOG.md) for migration examples.

## Features
* Very fast (see [benchmarks](#benchmarks) below).
* Compiled library (requires C++17 or later)
* Feature-rich formatting, using the excellent [fmt](https://github.com/fmtlib/fmt) library (bundled **12.1.0** by default; use `SPDLOG_FMT_EXTERNAL=ON` for a system **fmt**, **12.x** recommended).
* Asynchronous mode (optional)
* [Custom](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting) formatting.
* Multi/Single threaded loggers.
* Various log targets:
  * Rotating log files.
  * Daily log files.
  * Console logging (colors supported).
  * syslog.
  * Windows event log.
  * Windows debugger (```OutputDebugString(..)```).
  * Log to Qt widgets ([example](#log-to-qt-with-nice-colors)).
  * Easily [extendable](https://github.com/gabime/spdlog/wiki/4.-Sinks#implementing-your-own-sink) with custom log targets.
* Log filtering - log levels can be modified at runtime as well as compile time.

## Usage samples

#### Basic usage
```c++
#include "spdlog/spdlog.h"

int main() 
{
    spdlog::info("Welcome to spdlog!");
    spdlog::error("Some error message with arg: {}", 1);
    
    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    spdlog::info("Support for floats {:03.2f}", 1.23456);
    spdlog::info("Positional args are {1} {0}..", "too", "supported");
    spdlog::info("{:<30}", "left aligned");
    
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    spdlog::debug("This message should be displayed..");    
    
    // change log pattern
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    
    // Compile time log levels
    // define SPDLOG_ACTIVE_LEVEL to desired level
    SPDLOG_TRACE("Some trace message with param {}", 42);
    SPDLOG_DEBUG("Some debug message");
}

```
---
#### Create stdout/stderr logger object
```c++
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
void stdout_example()
{
    // Create color multithreading logger
    auto console = spdlog::create<spdlog::sinks::stdout_color_sink_mt>("console");
    // or for stderr:
    // auto console = spdlog::create<spdlog::sinks::stderr_color_sink_mt>("console");
}
```

---
#### Basic file logger
```c++
#include "spdlog/sinks/basic_file_sink.h"
void basic_logfile_example()
{
    try
    {
        auto logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("basic_logger", "logs/basic-log.txt", true);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::printf("Log init failed: %s\n", ex.what());
    }
}
```
---
#### Rotating files
```c++
#include "spdlog/sinks/rotating_file_sink.h"
void rotating_example()
{
    // Create a file rotating logger with 5 MB size max and 3 rotated files
    auto max_size = 1048576 * 5;
    auto max_files = 3;
    auto logger = spdlog::create<spdlog::sinks::rotating_file_sink_mt>("some_logger_name", "logs/rotating.txt", max_size, max_files);
}
```

---
#### Daily files
```c++
#include "spdlog/sinks/daily_file_sink.h"
void daily_example()
{
    // Create a daily logger - a new file is created every day at 2:30 am
    auto logger = spdlog::create<spdlog::sinks::daily_file_format_sink_mt>("daily_logger", "logs/daily.txt", 2, 30);
}
```


---
#### Flush
```c++
// Set flush level for the global logger - all messages with level >= warn will be flushed automatically
spdlog::flush_on(spdlog::level::warn);
```

---
#### Stopwatch
```c++
// Stopwatch support for spdlog
#include "spdlog/stopwatch.h"
void stopwatch_example()
{
    spdlog::stopwatch sw;    
    spdlog::debug("Elapsed {}", sw);
    spdlog::debug("Elapsed {:.3}", sw);       
}

```

---
#### Log binary data in hex
```c++
// many types of std::container<char> types can be used.
// ranges are supported too.
// format flags:
// {:X} - print in uppercase.
// {:s} - don't separate each byte with space.
// {:p} - don't print the position on each line start.
// {:n} - don't split the output into lines.
// {:a} - show ASCII if :n is not set.

#include "spdlog/bin_to_hex.h"

void binary_example()
{
    std::vector<char> buf;
    for (int i = 0; i < 80; i++) {
        buf.push_back(static_cast<char>(i & 0xff));
    }
    spdlog::info("Binary example: {}", spdlog::to_hex(buf));
    spdlog::info("Another binary example:{:n}", spdlog::to_hex(std::begin(buf), std::begin(buf) + 10));
    // more examples:
    // logger->info("uppercase: {:X}", spdlog::to_hex(buf));
    // logger->info("uppercase, no delimiters: {:Xs}", spdlog::to_hex(buf));
    // logger->info("uppercase, no delimiters, no position info: {:Xsp}", spdlog::to_hex(buf));
}
```

---
#### Logger with multi sinks - each with a different format and log level
```c++

// create a logger with 2 targets, with different log levels and formats.
// The console will show only warnings or errors, while the file will log all.
void multi_sink_example()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);

    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::debug);
    logger.warn("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
}
```

---
#### User-defined callbacks about log events
```c++
#include "spdlog/sinks/callback_sink.h"
// create a logger with a lambda function callback, the callback will be called
// each time something is logged to the logger
void callback_example()
{
    auto logger = spdlog::create<spdlog::sinks::callback_sink_mt>("custom_callback_logger",
        [](const spdlog::details::log_msg & /*msg*/) {
            // do what you need to do with msg
        });
}
```

---
#### Asynchronous logging
```c++
#include "spdlog/sinks/async_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
void async_example()
{
    using spdlog::sinks::async_sink;
    auto sink = async_sink::with<spdlog::sinks::basic_file_sink_mt>("logs/async_log.txt", true);
    auto logger = std::make_shared<spdlog::logger>("async_logger", sink);
    for (int i = 1; i < 101; ++i) {
        logger->info("Async message #{}", i);
    }
}
```

---
#### User-defined types
```c++
template<>
struct fmt::formatter<my_type> : fmt::formatter<std::string>
{
    auto format(my_type my, format_context &ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};

void user_defined_example()
{
    spdlog::info("user defined type: {}", my_type(14));
}

```

---
#### User-defined flags in the log pattern
```c++ 
// Log patterns can contain custom flags.
// the following example will add new flag '%*' - which will be bound to a <my_formatter_flag> instance.
#include "spdlog/pattern_formatter.h"
class my_formatter_flag : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        std::string some_txt = "custom-flag";
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return std::make_unique<my_formatter_flag>();
    }
};

void custom_flags_example()
{    
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<my_formatter_flag>('*').set_pattern("[%n] [%*] [%^%l%$] %v");
    spdlog::set_formatter(std::move(formatter));
}

```

---
#### Custom error handler
```c++
void err_handler_example()
{
    // can be set globally or per logger(logger->set_error_handler(..))
    spdlog::set_error_handler([](const std::string &msg) { printf("*** Custom log error handler: %s ***\n", msg.c_str()); });
}
```

---
#### syslog
```c++
#include "spdlog/sinks/syslog_sink.h"
void syslog_example()
{
    std::string ident = "spdlog-example";
    auto syslog_logger = spdlog::create<spdlog::sinks::syslog_sink_mt>("syslog", ident, LOG_PID);
    syslog_logger->warn("This is warning that will end up in syslog.");
}
```
---
#### Android example
```c++
#include "spdlog/sinks/android_sink.h"
void android_example()
{
    std::string tag = "spdlog-android";
    auto android_logger = spdlog::create<spdlog::sinks::android_sink_mt>("android", tag);
    android_logger->critical("Use \"adb shell logcat\" to view this message.");
}
```

---
#### Log file open/close event handlers
```c++
// You can get callbacks from spdlog before/after a log file has been opened or closed.
// This is useful for cleanup procedures or for adding something to the start/end of the log file.
void file_events_example()
{
    // pass the spdlog::file_event_handlers to file sinks for open/close log file notifications
    spdlog::file_event_handlers handlers;
    handlers.before_open = [](spdlog::filename_t) { spdlog::trace("Before opening logfile"); };
    handlers.after_open = [](spdlog::filename_t, std::FILE *fstream) { fputs("After opening\n", fstream); };
    handlers.before_close = [](spdlog::filename_t, std::FILE *fstream) { fputs("Before closing\n", fstream); };
    handlers.after_close = [](spdlog::filename_t) { spdlog::trace("After closing logfile"); };
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/events-sample.txt", true, handlers);
    spdlog::logger my_logger("some_logger", file_sink);
    my_logger.trace("Some log line");
}
```

---
#### Replace the Default Logger
```c++
void replace_global_logger_example()
{
    auto new_logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>("new_global_logger", "logs/new-default-log.txt", true);
    spdlog::set_global_logger(new_logger);
    spdlog::info("new logger log message");
}
```

---
#### Log to Qt with nice colors
```c++
#include "spdlog/spdlog.h"
#include "spdlog/sinks/qt_sinks.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setMinimumSize(640, 480);
    auto log_widget = new QTextEdit(this);
    setCentralWidget(log_widget);
    int max_lines = 500; // keep the text widget to max 500 lines. remove old lines if needed.
    auto logger = spdlog::create<spdlog::sinks::qt_color_sink_mt>("qt_logger", log_widget, max_lines);
    logger->info("Some info message");
}
```

---
## Benchmarks

Below are some [benchmarks](https://github.com/gabime/spdlog/blob/v2.x/bench/bench.cpp) done in Ubuntu 64 bit, Intel i7-4770 CPU @ 3.40GHz

#### Synchronous mode
```
[info] **************************************************************
[info] Single thread, 1,000,000 iterations
[info] **************************************************************
[info] basic_st         Elapsed: 0.17 secs        5,777,626/sec
[info] rotating_st      Elapsed: 0.18 secs        5,475,894/sec
[info] daily_st         Elapsed: 0.20 secs        5,062,659/sec
[info] empty_logger     Elapsed: 0.07 secs       14,127,300/sec
[info] **************************************************************
[info] C-string (400 bytes). Single thread, 1,000,000 iterations
[info] **************************************************************
[info] basic_st         Elapsed: 0.41 secs        2,412,483/sec
[info] rotating_st      Elapsed: 0.72 secs        1,389,196/sec
[info] daily_st         Elapsed: 0.42 secs        2,393,298/sec
[info] null_st          Elapsed: 0.04 secs       27,446,957/sec
[info] **************************************************************
[info] 10 threads, competing over the same logger object, 1,000,000 iterations
[info] **************************************************************
[info] basic_mt         Elapsed: 0.60 secs        1,659,613/sec
[info] rotating_mt      Elapsed: 0.62 secs        1,612,493/sec
[info] daily_mt         Elapsed: 0.61 secs        1,638,305/sec
[info] null_mt          Elapsed: 0.16 secs        6,272,758/sec
```
#### Asynchronous mode
```
[info] -------------------------------------------------
[info] Messages     : 1,000,000
[info] Threads      : 10
[info] Queue        : 8,192 slots
[info] Queue memory : 8,192 x 272 = 2,176 KB 
[info] -------------------------------------------------
[info] 
[info] *********************************
[info] Queue Overflow Policy: block
[info] *********************************
[info] Elapsed: 1.70784 secs     585,535/sec
[info] Elapsed: 1.69805 secs     588,910/sec
[info] Elapsed: 1.7026 secs      587,337/sec
[info] 
[info] *********************************
[info] Queue Overflow Policy: overrun
[info] *********************************
[info] Elapsed: 0.372816 secs    2,682,285/sec
[info] Elapsed: 0.379758 secs    2,633,255/sec
[info] Elapsed: 0.373532 secs    2,677,147/sec

```

## Documentation
Documentation can be found in the [wiki](https://github.com/gabime/spdlog/wiki/1.-QuickStart) pages.

---

Thanks to [JetBrains](https://www.jetbrains.com/?from=spdlog) for donating product licenses to help develop **spdlog** <a href="https://www.jetbrains.com/?from=spdlog"><img src="logos/jetbrains-variant-4.svg" width="94" align="center" /></a>
