// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// Writing to Windows Event Log requires the registry entries below to be present, with the following modifications:
// 1. {log_name}    should be replaced with your log name (e.g. your application name)
// 2. {source_name} should be replaced with the specific source name and the key should be duplicated for
//                  each source used in the application
// 
// Since typically modifications of this kind require elevation, it's better to do it as a part of setup procedure.
// However, the win_eventlog_sink constructor can do it for you in runtime if you set the message_file_path parameter to 
// win_eventlog_sink::DEFAULT_MESSAGE_FILE
// 
// You can also specify a custom message file if needed. 
// Please refer to Event Log functions descriptions in MSDN for more details on custom message files.

/*---------------------------------------------------------------------------------------

Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\{log_name}]

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\{log_name}\{source_name}]
"TypesSupported"=dword:00000007
"EventMessageFile"=hex(2):25,00,73,00,79,00,73,00,74,00,65,00,6d,00,72,00,6f,\
  00,6f,00,74,00,25,00,5c,00,53,00,79,00,73,00,74,00,65,00,6d,00,33,00,32,00,\
  5c,00,6d,00,73,00,63,00,6f,00,72,00,65,00,65,00,2e,00,64,00,6c,00,6c,00,00,\
  00

-----------------------------------------------------------------------------------------*/

#pragma once

#include <winbase.h>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>

#include <mutex>
#include <string>

namespace spdlog {
namespace sinks {

namespace win_eventlog {

namespace internal
{

struct utils
{
    /** Reports a message to stderr */
    static void report(char const* message) SPDLOG_NOEXCEPT
    {
        fprintf(stderr, "%s", message);
        fflush(stderr);
    }

    /** Reports a message to stderr */
    static void report(std::string const& message) SPDLOG_NOEXCEPT
    {
        report(message.c_str());
    }
};

/** Windows error */
struct win32_error : public spdlog_ex
{
    /** Formats an error report line: "user-message: error-code (system message)" */
    static std::string format(std::string const& user_message, DWORD error_code = GetLastError())
    {
        std::string system_message;

        LPSTR format_message_result {};
        auto format_message_succeeded = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
            error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &format_message_result, 0, nullptr);

        if (format_message_succeeded && format_message_result)
        {
            system_message = fmt::format(" ({})", format_message_result);
        }

        if (format_message_result)
            LocalFree((HLOCAL) format_message_result);

        return fmt::format("{}: {}{}", user_message, error_code, system_message);
    }

    win32_error(std::string const& func_name, DWORD error = GetLastError())
        : spdlog_ex(format(func_name, error))
    {}
};

/** Wrapper for security identifiers (SID) on Windows */
struct sid_t
{
    std::vector<char> buffer_;

public:
    sid_t()
    {}

    /** creates a wrapped SID copy */
    static sid_t duplicate_sid(PSID psid)
    {
        if (!IsValidSid(psid))
            SPDLOG_THROW(spdlog_ex("sid_t::sid_t(): invalid SID received"));

        auto const sid_length {::GetLengthSid(psid)};

        sid_t result;
        result.buffer_.resize(sid_length);
        if (!CopySid(sid_length, (PSID) result.as_sid(), psid))
            SPDLOG_THROW(win32_error("CopySid"));

        return result;
    }

    /** Retrieves pointer to the internal buffer contents as SID* */
    SID * as_sid() const
    {
        return buffer_.empty() ? nullptr : (SID *) buffer_.data();
    }

    /** Get SID for the current user */
    static sid_t get_current_user_sid()
    {
        /* create and init RAII holder for process token */
        struct process_token_t
        {
            HANDLE hToken_;
            bool hasToken_;

            process_token_t(HANDLE process)
                : hToken_(0)
                , hasToken_(OpenProcessToken(process, TOKEN_QUERY, &hToken_))
            {
                if (!hasToken_)
                    SPDLOG_THROW(win32_error("OpenProcessToken"));
            }

            ~process_token_t()
            {
                if (hasToken_ && !CloseHandle(hToken_))
                    utils::report(win32_error::format("CloseHandle"));
            }
        } current_process_token(GetCurrentProcess()); // GetCurrentProcess returns pseudohandle, no leak here!

        // Get the required size, this is expected to fail with ERROR_INSUFFICIENT_BUFFER and return the token size
        DWORD tusize = 0;
        GetTokenInformation(current_process_token.hToken_, TokenUser, NULL, 0, &tusize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            SPDLOG_THROW(win32_error("GetTokenInformation"));

        // get user token
        std::vector<unsigned char> buffer(tusize);
        if (!GetTokenInformation(current_process_token.hToken_, TokenUser, (LPVOID) buffer.data(), tusize, &tusize))
            SPDLOG_THROW(win32_error("GetTokenInformation"));

        // create a wrapper of the SID data as stored in the user token
        return sid_t::duplicate_sid(((TOKEN_USER*) buffer.data())->User.Sid);
    }
};

struct eventlog
{
    static WORD get_event_type(details::log_msg const& msg)
    {
        switch (msg.level)
        {
            case level::trace:
            case level::debug:
                return EVENTLOG_SUCCESS;

            case level::info:
                return EVENTLOG_INFORMATION_TYPE;

            case level::warn:
                return EVENTLOG_WARNING_TYPE;

            case level::err:
            case level::critical:
            case level::off:
                return EVENTLOG_ERROR_TYPE;

            default:
                // should be unreachable
                SPDLOG_THROW(std::logic_error(fmt::format("Unsupported log level {}", msg.level)));
        }
    }

    static WORD get_event_category(details::log_msg const& msg)
    {
        return (WORD) msg.level;
    }
};

} // namespace internal


/*
 * Windows Event Log sink
 */
template <typename Mutex>
class win_eventlog_sink : public base_sink<Mutex>
{
private:
    HANDLE hEventLog_ {NULL};
    internal::sid_t current_user_sid_;
    std::string source_;
    WORD event_id_ {DEFAULT_EVENT_ID};

    HANDLE event_log_handle()
    {
        if (!hEventLog_)
        {
            hEventLog_ = RegisterEventSource(nullptr, source_.c_str());
            if (!hEventLog_ || hEventLog_ == (HANDLE) ERROR_ACCESS_DENIED)
                SPDLOG_THROW(internal::win32_error("RegisterEventSource"));
        }

        return hEventLog_;
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        using namespace internal;

        memory_buf_t formatted;
        formatter_->format(msg, formatted);

        auto formatted_string = fmt::to_string(formatted);
        auto formatted_string_lpsz = formatted_string.c_str();

        bool succeeded = ReportEvent(
            event_log_handle(),
            eventlog::get_event_type(msg),
            eventlog::get_event_category(msg),
            event_id_,
            current_user_sid_.as_sid(),
            1,
            0,
            &formatted_string_lpsz,
            nullptr);

        if (!succeeded)
            SPDLOG_THROW(win32_error("ReportEvent"));
    }

    void flush_() override {}

public:
    static const std::string DEFAULT_MESSAGE_FILE;
    static const WORD DEFAULT_EVENT_ID {1000};

    win_eventlog_sink( std::string const& source ) 
        : source_(source)
    {
        using namespace internal;
        try
        {
            current_user_sid_ = sid_t::get_current_user_sid();
        }
        catch (std::exception const& e)
        {
            utils::report(e.what());
        }
    }

    ~win_eventlog_sink()
    {
        using namespace internal;

        if (hEventLog_ && !DeregisterEventSource(hEventLog_))
            utils::report(win32_error::format("DeregisterEventSource"));
    }

    /** 
    Register the log source in the Windows registry.

    Requires elevation on Windows Vista and later.
    */
    void add_registry_info(std::string const& log = "Application", std::string const& message_file_path = DEFAULT_MESSAGE_FILE, WORD event_id = DEFAULT_EVENT_ID) 
    {
        using namespace internal;

        event_id_ = event_id;

        std::string logSourceRegKeyName = fmt::format("SYSTEM\\CurrentControlSet\\Services\\EventLog\\{}\\{}", log, source_);

        struct hkey_t
        {
            ::HKEY handle_ {};

            ~hkey_t()
            {
                if (handle_)
                    RegCloseKey(handle_);
            }
        } logSourceRegKey;

        DWORD disposition {};
        long stat = RegCreateKeyEx(HKEY_LOCAL_MACHINE, logSourceRegKeyName.c_str(), 0, NULL,
                                   REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                                   &logSourceRegKey.handle_, &disposition);
        if (stat == ERROR_SUCCESS)
        {
            if (disposition == REG_CREATED_NEW_KEY && !message_file_path.empty())
            {
                auto const expanded_message_file_path_length = ExpandEnvironmentStrings(message_file_path.c_str(), (LPSTR) &disposition, 0);
                if (!expanded_message_file_path_length)
                    SPDLOG_THROW(win32_error("ExpandEnvironmentStrings"));

                std::vector<char>expanded_message_file_path(expanded_message_file_path_length);
                ExpandEnvironmentStrings(message_file_path.c_str(), expanded_message_file_path.data(), expanded_message_file_path_length); // this can't fail if the preivous ExpandEnvironmentStrings succeeded

                stat = RegSetValueEx(logSourceRegKey.handle_, "EventMessageFile", 0, REG_SZ, (LPBYTE) expanded_message_file_path.data(), expanded_message_file_path_length);
                if (stat != ERROR_SUCCESS)
                    SPDLOG_THROW(win32_error("RegSetValueEx", stat));

                DWORD typesSupported = 7;
                stat = RegSetValueEx(logSourceRegKey.handle_, "TypesSupported", 0, REG_DWORD, (LPBYTE) &typesSupported, sizeof(DWORD));
                if (stat != ERROR_SUCCESS)
                    SPDLOG_THROW(win32_error("RegSetValueEx", stat));
            }
        }
        else
        {
            SPDLOG_THROW(win32_error("RegCreateKeyEx", stat));
        }
    }

};

template <typename Mutex>
const std::string win_eventlog_sink<Mutex>::DEFAULT_MESSAGE_FILE = "%systemroot%\\system32\\mscoree.dll";

} // namespace win_eventlog

using win_eventlog_sink_mt = win_eventlog::win_eventlog_sink<std::mutex>;
using win_eventlog_sink_st = win_eventlog::win_eventlog_sink<details::null_mutex>;

} // namespace sinks
} // namespace spdlog

