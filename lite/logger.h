//
// Created by gabi on 3/16/19.
//

#ifndef SPDLOG_LIB_LOGGER_H
#define SPDLOG_LIB_LOGGER_H

#include <memory>
#include "spd_types.h"
#include "spdlog/fmt/fmt.h"


namespace spdlog {
    class logger;

    namespace lite {
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

//        template<typename... Args>
//        void log(spdlog::lite::level lvl, const char *fmt, const Args &... args)
//        {
//            log(lvl, fmt, args...);
//        }

            template<typename... Args>
            void trace(const char *fmt, const Args &... args) {
                log(spdlog::lite::level::trace, fmt, args...);
            }

        protected:
            std::shared_ptr<spdlog::logger> impl_;

            void log_formatted_(spdlog::lite::level lvl, const fmt::memory_buffer &formatted);

        };

    } // namespace lite

    // factory to create lite logger
    // implement it in a dedicated compilation unit for fast compiles
    spdlog::lite::logger create_lite(void* ctx = nullptr);
} // namespace spdlog
#endif //SPDLOG_LIB_LOGGER_H
