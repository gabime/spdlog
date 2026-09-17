//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog C++20 module usage example.
//
// This mirrors example.cpp, but consumes spdlog through `import spdlog;`
// instead of #including its headers. It is deliberately smaller: the module
// exports the core API and the dependency-free sinks only, so the syslog/udp
// parts of example.cpp have no counterpart here (those sinks stay header-only
// and can simply be #included next to the import).

// IMPORTANT: all #includes come *before* the import. MSVC reports a cascade of
// C2572 "redefinition of default argument" errors inside its standard library
// when a standard header is included after importing a module whose global
// module fragment already included it.
#include <cstdio>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/macros.h>  // SPDLOG_INFO(..) and friends; a module cannot export macros
#include <spdlog/version.h>  // SPDLOG_VER_MAJOR and friends are macros too

// The fmt names needed to specialize fmt::formatter<T> below are re-exported by the
// module. std::formatter is not (names from namespace std never are), so the
// std::format build includes <format> itself.
#if defined(SPDLOG_USE_STD_FORMAT)
#include <format>  // std::formatter; names from namespace std are never re-exported
#endif

// Do NOT also #include <spdlog/mdc.h> (or any other <spdlog/...> header) here: unlike the
// companion wrapper-module design, this native module attaches every spdlog declaration to
// the named module `spdlog`, so a plain #include of the same header creates a *second*,
// distinct copy of the same names attached to the global module instead - two different
// spdlog::mdc, spdlog::logger, etc. with the same spelling. Linking both into one program is
// an ODR violation (observed as "duplicate symbol" at link time with Clang/lld). See INSTALL
// for the full explanation.
import spdlog;

void stdout_logger_example();
void basic_example();
void rotating_example();
void daily_example();
void callback_example();
void async_example();
void binary_example();
void stopwatch_example();
void multi_sink_example();
void user_defined_example();
void err_handler_example();
void custom_flags_example();
void file_events_example();
void replace_default_logger_example();
void mdc_example();

int main(int, char *[]) {
    try {
        // Log levels can be loaded from the environment using "SPDLOG_LEVEL"
        spdlog::cfg::load_env_levels();

        spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR,
                     SPDLOG_VER_PATCH);

        spdlog::warn("Easy padding in numbers like {:08d}", 12);
        spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        spdlog::info("Support for floats {:03.2f}", 1.23456);
        spdlog::info("Positional args are {1} {0}..", "too", "supported");
        spdlog::info("{:>8} aligned, {:<8} aligned", "right", "left");

        // Runtime log levels
        spdlog::set_level(spdlog::level::info);
        spdlog::debug("This message should not be displayed!");
        spdlog::set_level(spdlog::level::trace);
        spdlog::debug("This message should be displayed..");

        // Customize msg format for all loggers
        spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [thread %t] %v");
        spdlog::info("This an info message with custom format");
        spdlog::set_pattern("%+");  // back to default format
        spdlog::set_level(spdlog::level::info);

        // The SPDLOG_* macros come from <spdlog/macros.h>
        SPDLOG_TRACE("Some trace message with param {}", 42);
        SPDLOG_DEBUG("Some debug message");
        SPDLOG_INFO("Some info message from a macro");

        stdout_logger_example();
        basic_example();
        rotating_example();
        daily_example();
        callback_example();
        async_example();
        binary_example();
        multi_sink_example();
        user_defined_example();
        err_handler_example();
        file_events_example();
        replace_default_logger_example();
        mdc_example();
        stopwatch_example();
        custom_flags_example();

        // Release all spdlog resources, and drop all loggers in the registry.
        // This is optional (only mandatory if using windows + async log).
        spdlog::shutdown();
    }

    // Exceptions will only be thrown upon failed logger or sink construction
    // (not during logging).
    catch (const spdlog::spdlog_ex &ex) {
        std::printf("Log initialization failed: %s\n", ex.what());
        return 1;
    }
}

void stdout_logger_example() {
    // Create color multi-threaded logger
    auto console = spdlog::stdout_color_mt("console");
    // or for stderr:
    // auto console = spdlog::stderr_color_mt("error-logger");
    console->info("Hello from a module-imported color logger");
}

void basic_example() {
    // Create basic file logger (not rotated).
    auto my_logger = spdlog::basic_logger_mt("file_logger", "logs/basic-log.txt", true);
    my_logger->info("Hello from a module-imported basic logger");
}

void rotating_example() {
    // Create a file rotating logger with 5 MB size max and 3 rotated files.
    auto rotating_logger =
        spdlog::rotating_logger_mt("some_logger_name", "logs/rotating.txt", 1048576 * 5, 3);
    rotating_logger->info("Hello from a module-imported rotating logger");
}

void daily_example() {
    // Create a daily logger - a new file is created every day at 2:30 am.
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/daily.txt", 2, 30);
    daily_logger->info("Hello from a module-imported daily logger");
}

void callback_example() {
    // Create the logger
    auto logger = spdlog::callback_logger_mt("custom_callback_logger",
                                             [](const spdlog::details::log_msg & /*msg*/) {
                                                 // do what you need to do with msg
                                             });
    logger->info("Hello from a module-imported callback logger");
}

void async_example() {
    // Default thread pool settings can be modified *before* creating the
    // async logger:
    // spdlog::init_thread_pool(32768, 1); // queue with max 32k items 1 backing thread.
    auto async_file =
        spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");

    for (int i = 1; i < 11; ++i) {
        async_file->info("Async message #{}", i);
    }
}

// Log binary data as hex.
void binary_example() {
#if !defined(SPDLOG_USE_STD_FORMAT) || defined(_MSC_VER)
    std::vector<char> buf(80);
    for (int i = 0; i < 80; i++) {
        buf.push_back(static_cast<char>(i & 0xff));
    }
    spdlog::info("Binary example: {}", spdlog::to_hex(buf));
    spdlog::info("Another binary example:{:n}",
                 spdlog::to_hex(std::begin(buf), std::begin(buf) + 10));
#endif
}

void stopwatch_example() {
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
    spdlog::info("Stopwatch: {} seconds", sw);
}

// A logger with multiple sinks (stdout and file) - each with a different format
// and log level.
void multi_sink_example() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::warn);
    console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
    file_sink->set_level(spdlog::level::trace);

    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::debug);
    logger.warn("this should appear in both console and file");
    logger.info("this message should not appear in the console, only in the file");
}

// User defined types logging
struct my_type {
    int i = 0;
    explicit my_type(int i)
        : i(i) {}
};

#if !defined(SPDLOG_USE_STD_FORMAT)  // fmt formatter; fmt::formatter comes from our own #include above
template <>
struct fmt::formatter<my_type> : fmt::formatter<std::string> {
    auto format(my_type my, fmt::format_context &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};
#else  // std::format formatter; std::formatter comes from our own #include above
template <>
struct std::formatter<my_type> : std::formatter<std::string> {
    auto format(my_type my, std::format_context &ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[my_type i={}]", my.i);
    }
};
#endif

void user_defined_example() { spdlog::info("user defined type: {}", my_type(14)); }

// Custom error handler. Will be triggered on log failure.
void err_handler_example() {
    spdlog::set_error_handler([](const std::string &msg) {
        printf("*** Custom log error handler: %s ***\n", msg.c_str());
    });
}

// Custom formatter flag: prints %^ as "custom-flag"
class my_formatter_flag : public spdlog::custom_flag_formatter {
public:
    void format(const spdlog::details::log_msg &,
                const std::tm &,
                spdlog::memory_buf_t &dest) override {
        std::string some_txt = "custom-flag";
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override {
        return spdlog::details::make_unique<my_formatter_flag>();
    }
};

void custom_flags_example() {
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<my_formatter_flag>('*').set_pattern("[%n] [%*] [%^%l%$] %v");
    // set the new formatter using spdlog::set_formatter(formatter) or
    // logger->set_formatter(formatter)
    spdlog::set_formatter(std::move(formatter));
    spdlog::info("custom flag example");
    spdlog::set_pattern("%+");  // back to default format
}

void file_events_example() {
    // pass the spdlog::file_event_handlers to file sinks for open/close log file notifications
    spdlog::file_event_handlers handlers;
    handlers.before_open = [](spdlog::filename_t filename) {
        spdlog::info("Before opening {}", filename);
    };
    handlers.after_open = [](spdlog::filename_t filename, std::FILE *fstream) {
        spdlog::info("After opening {}", filename);
        fputs("After opening\n", fstream);
    };
    handlers.before_close = [](spdlog::filename_t filename, std::FILE *fstream) {
        spdlog::info("Before closing {}", filename);
        fputs("Before closing\n", fstream);
    };
    handlers.after_close = [](spdlog::filename_t filename) {
        spdlog::info("After closing {}", filename);
    };
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/events-sample.txt",
                                                                        true, handlers);
    spdlog::logger my_logger("some_logger", file_sink);
    my_logger.info("Some log line");
}

void replace_default_logger_example() {
    auto new_logger =
        spdlog::basic_logger_mt("new_default_logger", "logs/new-default-log.txt", true);
    auto old_logger = spdlog::default_logger();
    spdlog::set_default_logger(new_logger);
    spdlog::info("new logger log message");
    spdlog::set_default_logger(old_logger);
}

void mdc_example() {
#ifndef SPDLOG_NO_TLS
    spdlog::mdc::put("key1", "value1");
    spdlog::mdc::put("key2", "value2");
    // if not using the default format, you can use the %& formatter to print mdc data
    spdlog::set_pattern("[%H:%M:%S %z] [%^%L%$] [%&] %v");
    spdlog::info("Some log message with context");
    spdlog::set_pattern("%+");  // back to default format
#endif
}
