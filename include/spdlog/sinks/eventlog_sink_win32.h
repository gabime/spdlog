// Copyright(c) 2020 bandana2004@gmail.com
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

namespace eventlog_sink_win32 {

struct win32_error : public spdlog_ex
{
    static std::string format(std::string const& func, DWORD error = GetLastError())
    {
        return fmt::format(func + " failed ({0})", error);
    }

    static void report(char const* message) SPDLOG_NOEXCEPT
    {
        fprintf(stderr, "%s", message);
        fflush(stderr);
    }

    static void report(std::string const& message) SPDLOG_NOEXCEPT
    {
        report(message.c_str());
    }

    win32_error(std::string const& func_name, DWORD error = GetLastError())
        : spdlog_ex(format(func_name, error), error)
    {}
};

struct sid
{
    static SID* duplicate(SID* p_src_sid)
    {
        if (!IsValidSid(p_src_sid))
            SPDLOG_THROW(spdlog_ex("sid::duplicate: invalid SID received"));

        DWORD sid_size = ::GetLengthSid(p_src_sid);
        SID* p_dest_sid = (SID*) ::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sid_size);

        if (!p_dest_sid)
            SPDLOG_THROW(win32_error("HeapAlloc"));

        if (!CopySid(sid_size, p_dest_sid, p_src_sid))
        {
            sid::free(&p_dest_sid);
            SPDLOG_THROW(win32_error("CopySid"));
        }

        return p_dest_sid;
    }

    static void free(SID** ppsid)
    {
        if (ppsid && *ppsid)
        {
            if (!HeapFree(GetProcessHeap(), 0, (LPVOID) *ppsid))
            {
                SPDLOG_THROW(win32_error("HeapFree"));
            }
        }

        *ppsid = nullptr;
    }

    static SID* get_current_user_sid()
    {
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
                    win32_error::report(win32_error::format("CloseHandle"));
            }
        } current_process_token(GetCurrentProcess()); // GetCurrentProcess returns pseudohandle, no leak here!

        // Get the required size
        DWORD tusize = 0;
        GetTokenInformation(current_process_token.hToken_, TokenUser, NULL, 0, &tusize);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            SPDLOG_THROW(win32_error("GetTokenInformation"));

        std::vector<unsigned char> buffer(tusize);
        if (!GetTokenInformation(current_process_token.hToken_, TokenUser, (LPVOID) buffer.data(), tusize, &tusize))
            SPDLOG_THROW(win32_error("GetTokenInformation"));

        return sid::duplicate((SID *) ((TOKEN_USER*) buffer.data())->User.Sid);
    }
};

struct eventlog
{
    static WORD get_event_type(details::log_msg const& msg)
    {
        WORD type = EVENTLOG_SUCCESS;
        if (msg.level >= level::info)
        {
            type = EVENTLOG_INFORMATION_TYPE;
            if (msg.level >= level::warn)
            {
                type = EVENTLOG_WARNING_TYPE;
                if (msg.level >= level::err)
                {
                    type = EVENTLOG_ERROR_TYPE;
                }
            }
        }
        return type;
    }

    static WORD get_event_category(details::log_msg const& msg)
    {
        WORD category = 1;
        if (msg.level >= level::debug)
        {
            category = 2;
            if (msg.level >= level::info)
            {
                category = 3;
                if (msg.level >= level::warn)
                {
                    category = 4;
                    if (msg.level >= level::err)
                    {
                        category = 5;
                        if (msg.level >= level::critical)
                        {
                            category = 6;
                        }
                    }
                }
            }
        }
        return category;
    }
};

class sink : public base_sink<spdlog::details::null_mutex>
{
private:
    HANDLE hEventLog_;
    SID* current_user_sid_;
    std::string source_;
    std::string log_;

    void add_registry_info(std::string const& message_file_path)
    {
        std::string subkey("SYSTEM\\CurrentControlSet\\Services\\EventLog\\");
        subkey += log_ + "\\" + source_;

        ::HKEY hkey {};
        DWORD disposition {};
        long stat = RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, NULL,
                                   REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL,
                                   &hkey, &disposition);
        if (stat == ERROR_SUCCESS)
        {
            if (disposition == REG_CREATED_NEW_KEY && !message_file_path.empty())
            {
                auto const expanded_message_file_path_length = ExpandEnvironmentStrings(message_file_path.c_str(), (LPSTR) &disposition, 0);
                std::vector<char>expanded_message_file_path(expanded_message_file_path_length + 1);
                ExpandEnvironmentStrings(message_file_path.c_str(), expanded_message_file_path.data(), expanded_message_file_path_length);

                RegSetValueEx(hkey, "EventMessageFile", 0, REG_SZ, (LPBYTE) expanded_message_file_path.data(), expanded_message_file_path_length);
                DWORD typesSupported = 7;
                RegSetValueEx(hkey, "TypesSupported", 0, REG_DWORD, (LPBYTE) &typesSupported, sizeof(DWORD));
            }
        }
        else
        {
            SPDLOG_THROW(win32_error("RegCreateKeyEx", stat));
        }

        RegCloseKey(hkey);
    }

protected:
    void sink_it_(const details::log_msg &msg) override
    {
        memory_buf_t formatted;
        formatter_->format(msg, formatted);

        auto formatted_string = fmt::to_string(formatted);
        auto formatted_string_lpsz = formatted_string.c_str();

        if (!ReportEvent(hEventLog_, eventlog::get_event_type(msg), eventlog::get_event_category(msg), 1000, current_user_sid_, 1, 0, &formatted_string_lpsz, nullptr))
            SPDLOG_THROW(win32_error("ReportEvent"));
    }

    void flush_() override {}

public:
    sink(
        std::string const& source, 
        std::string const& log, 
        std::string const& message_file_path) 
        : source_(source)
        , log_(log)
    {
        if (!message_file_path.empty())
            add_registry_info(message_file_path);

        current_user_sid_ = sid::get_current_user_sid();
        hEventLog_ = RegisterEventSource(nullptr, source.c_str());
        if (!hEventLog_)
            SPDLOG_THROW(win32_error("RegisterEventSource"));
    }

    ~sink()
    {
        try
        {
            if (hEventLog_ && !DeregisterEventSource(hEventLog_))
                win32_error::report(win32_error::format("DeregisterEventSource"));

            sid::free(&current_user_sid_);
        }
        catch (std::exception const& e)
        {
            win32_error::report(e.what());
        }
    }
};

} // namespace eventlog_sink_win32

template <typename Mutex>
eventlog_sink<Mutex>::eventlog_sink(std::string const& source, std::string const& log, std::string const& message_file_path)
    : impl_(std::make_unique<eventlog_sink_win32::sink>(source, log, message_file_path))
{}

template <typename Mutex>
void eventlog_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    impl_->log(msg);
}

