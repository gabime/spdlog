// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// Requires the following registry entries to be present, with the following modifications:
// 1. {app_name} should be replaced with your application name
// 2. {log_name} should be replaced with the specific log name and the key should be duplicated for
//               each log used in the application

/*---------------------------------------------------------------------------------------

Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\{app_name}]

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\{app_name}\{log_name}]
"TypesSupported"=dword:00000007
"EventMessageFile"=hex(2):25,00,73,00,79,00,73,00,74,00,65,00,6d,00,72,00,6f,\
  00,6f,00,74,00,25,00,5c,00,53,00,79,00,73,00,74,00,65,00,6d,00,33,00,32,00,\
  5c,00,6d,00,73,00,63,00,6f,00,72,00,65,00,65,00,2e,00,64,00,6c,00,6c,00,00,\
  00

-----------------------------------------------------------------------------------------*/

#pragma once

#ifdef _WIN32
#include <winbase.h>
#endif

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {
/*
 * Windows Event Log sink
 */
template<typename Mutex>
class eventlog_sink : public base_sink<Mutex>
    {
    public:
        explicit eventlog_sink(
            std::string const& source,
            std::string const& log = "Application",
            std::string const& message_file_path = "%windir%\\System32\\mscoree.dll");

        eventlog_sink(eventlog_sink const&) = delete;
        eventlog_sink& operator=(eventlog_sink const&) = delete;
    protected:
        void sink_it_(const details::log_msg &msg) override;
        void flush_() override {}
        virtual void set_pattern_(const std::string &pattern) override;
        virtual void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) override;

    private:
        std::unique_ptr<sink> impl_;
    };

#ifdef _WIN32
#include "eventlog_sink_win32.h"
#else

template <typename Mutex>
eventlog_sink<Mutex>::eventlog_sink(std::string const& source, std::string const& log, std::string const& message_file_path)
    {
    }

template <typename Mutex>
void eventlog_sink<Mutex>::sink_it_(const details::log_msg &msg)
    {}


#endif

template <typename Mutex>
void eventlog_sink<Mutex>::set_pattern_(const std::string &pattern)
{
    if (impl_)
        impl_->set_pattern(pattern);
}

template <typename Mutex>
void eventlog_sink<Mutex>::set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter)
{
    if (impl_)
        impl_->set_formatter(std::move(sink_formatter));
}

using eventlog_sink_mt = eventlog_sink<std::mutex>;
using eventlog_sink_st = eventlog_sink<details::null_mutex>;

using windebug_sink_mt = eventlog_sink_mt;
using windebug_sink_st = eventlog_sink_st;

} // namespace sinks
} // namespace spdlog

