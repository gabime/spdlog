//
// Created by gabi on 3/16/19.
//
#pragma once

#include <memory>
#include <string>
#include "spdlog/fmt/fmt.h"


//#define SPDLITE_LOGGER_INFO(logger, ...) SPDLITE_LOGGER_CALL(logger, spdlog::lite::level::info, __VA_ARGS__)
//#define SPDLITE_INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)

namespace spdlog {
    class logger;

    namespace lite {
        enum class level{
            trace,
            debug,
            info,
            warn,
            err,
            critical,
            off
        };

        struct src_loc {
            const char *filename;
            int line;
            const char* funcname;
        };

        class logger {
        public:
            logger() = default;

            logger(std::shared_ptr<spdlog::logger> impl);
            logger(const logger&) = default;
            logger(logger&&) = default;
            logger& operator=(const logger&) = default;

            ~logger() = default;

            bool should_log(spdlog::lite::level lvl) const noexcept;

            template<typename... Args>
            void log(spdlog::lite::level lvl, const char *fmt, const Args &... args) {
                if (!should_log(lvl)) {
                    return;
                }
                fmt::memory_buffer formatted_buf;
                fmt::format_to(formatted_buf, fmt, args...);
                log_formatted_(lvl, formatted_buf);
            }


            template<typename... Args>
            void log(const spdlog::lite::src_loc& src, spdlog::lite::level lvl, const char *fmt, const Args &... args) {
                if (!should_log(lvl)) {
                    return;
                }
                fmt::memory_buffer formatted_buf;
                fmt::format_to(formatted_buf, fmt, args...);
                log_formatted_src(src, lvl, formatted_buf);
            }


            template<typename... Args>
            void trace(const char *fmt, const Args &... args)
            {
                log(spdlog::lite::level::trace, fmt, args...);
            }

            template<typename... Args>
            void trace(const char* source_file, int source_line, const char* source_func, const char *fmt, const Args &... args)
            {
                spdlog::lite::src_loc src{source_file, source_line, source_func};
                log(src, spdlog::lite::level::trace, fmt, args...);
            }

            template<typename... Args>
            void debug(const char *fmt, const Args &... args)
            {
                log(spdlog::lite::level::debug, fmt, args...);
            }

            template<typename... Args>
            void info(const char *fmt, const Args &... args)
            {
                log(spdlog::lite::level::info, fmt, args...);
            }

            template<typename... Args>
            void warn(const char *fmt, const Args &... args)
            {
                log(spdlog::lite::level::warn, fmt, args...);
            }

            template<typename... Args>
            void error(const char *fmt, const Args &... args)
            {
                log(spdlog::lite::level::err, fmt, args...);
            }

            template<typename... Args>
            void critical(const char *fmt, const Args &... args)
            {
                log(spdlog::lite::level::critical, fmt, args...);
            }

            std::string name() const;

            // level
            void set_level(lite::level level);
            lite::level get_level() const;

            // flush
            void flush();
            void flush_on(spdlog::lite::level log_level);
            spdlog::lite::level flush_level() const;

            // pattern
            void set_pattern(std::string pattern);

        protected:
            std::shared_ptr<spdlog::logger> impl_;
            void log_formatted_(spdlog::lite::level lvl, const fmt::memory_buffer &formatted);
            void log_formatted_src(const spdlog::lite::src_loc& src, spdlog::lite::level lvl, const fmt::memory_buffer &formatted);

        };
        spdlog::lite::logger& default_logger();


        template<typename... Args>
        void trace(const char *fmt, const Args &... args)
        {
            default_logger().trace(fmt, args...);
        }
        template<typename... Args>
        void debug(const char *fmt, const Args &... args)
        {
            default_logger().debug(fmt, args...);
        }

        template<typename... Args>
        void info(const char *fmt, const Args &... args)
        {
            default_logger().info(fmt, args...);
        }

        template<typename... Args>
        void warn(const char *fmt, const Args &... args)
        {
            default_logger().warn(fmt, args...);
        }

        template<typename... Args>
        void error(const char *fmt, const Args &... args)
        {
            default_logger().error(fmt, args...);
        }

        template<typename... Args>
        void critical(const char *fmt, const Args &... args)
        {
            default_logger().critical(fmt, args...);
        }


    } // namespace lite

    // factory to create lite logger
    // implement it in a dedicated compilation unit for fast compiles
    spdlog::lite::logger create_lite(void* ctx = nullptr);

} // namespace spdlog