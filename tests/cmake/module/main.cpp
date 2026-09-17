// Consumer of the installed C++20 module. Exercises enough of the exported
// surface that a missing SPDLOG_EXPORT in the headers shows up here.
//
// All #includes come before the import: MSVC rejects a standard header that is
// included after importing a module whose global module fragment had it too.
//
// fmt (or std::format) stays attached to the *global* module, not to spdlog, so
// fmt::formatter is not reachable through `import spdlog;` by qualified name;
// #including it directly below is fine (ordinary, non-modular third-party use). Do
// NOT, however, also #include a header that declares spdlog's *own* entities (e.g.
// <spdlog/mdc.h>): this native module attaches those to the named module `spdlog`,
// so #including one on top of importing creates a second, distinct copy of the same
// names (ODR violation).
//
// <chrono> in particular: pattern_formatter's default constructor (used below through
// details::make_unique) pulls in details::elapsed_formatter<Padder, std::chrono::seconds>,
// a template whose point of instantiation can land in this consumer's own TU. MSVC's
// module implementation does not treat std::chrono::duration's comparison operators -
// reachable only through spdlog.cppm's global module fragment - as reachable from that
// instantiation, and fails with "no operator < found" unless this TU has its own
// #include <chrono> too. Harmless and portable to include unconditionally.
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/macros.h>
#if defined(SPDLOG_USE_STD_FORMAT)
#include <format>
#else
#include <spdlog/fmt/fmt.h>
#endif

import spdlog;

struct my_type {
    int i = 0;
};

#if !defined(SPDLOG_USE_STD_FORMAT)
template <>
struct fmt::formatter<my_type> : fmt::formatter<std::string> {
    auto format(my_type m, fmt::format_context &ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "[my_type i={}]", m.i);
    }
};
#else
template <>
struct std::formatter<my_type> : std::formatter<std::string> {
    auto format(my_type m, std::format_context &ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "[my_type i={}]", m.i);
    }
};
#endif

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("module consumer: {} {}", 1, "two");
    spdlog::info("user defined type: {}", my_type{7});

    // stopwatch exercises the formatter specialization kept alive in the purview
    spdlog::stopwatch sw;
    spdlog::info("elapsed {}", sw);

    // macros come from <spdlog/macros.h>, not from the module
    SPDLOG_INFO("from a macro");
    SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), "from a logger macro");

    // a sink built by hand, plus a factory function
    auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    spdlog::logger logger("module_logger", sink);
    logger.set_level(spdlog::level::trace);
    logger.trace("hello");

    auto null_logger = spdlog::null_logger_mt("null");
    null_logger->info("hello");

    // async factory: references details::default_async_q_size, which is compiled
    // into this module's purview - it needs external (inline-variable) linkage or
    // referencing it from the module's exported async_factory_impl would be an
    // ill-formed "exposure" of a TU-local entity
    auto async_null = spdlog::null_logger_mt<spdlog::async_factory>("async_null");
    async_null->info("hello");

    // pattern_formatter default argument references details::os::default_eol
    auto formatter = spdlog::details::make_unique<spdlog::pattern_formatter>();
    spdlog::set_formatter(std::move(formatter));

    spdlog::mdc::put("k", "v");

    try {
        throw spdlog::spdlog_ex("expected");
    } catch (const spdlog::spdlog_ex &) {
    }

    spdlog::shutdown();
    return 0;
}
