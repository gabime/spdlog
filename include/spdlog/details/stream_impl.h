//
// Copyright(c) 2017 Benoit Leforestier.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

template <class CharT, class Traits>
spdlog::basic_streambuf<CharT, Traits>::basic_streambuf(spdlog::logger* plogger, spdlog::level::level_enum lvl) :
    m_plogger(plogger),
    m_log_msg(nullptr, lvl)
{
    if (m_plogger)
    {
        const std::string& name = m_plogger->name();
        m_log_msg.logger_name = &name;
    }
}

template <class CharT, class Traits>
spdlog::basic_streambuf<CharT, Traits>::basic_streambuf(basic_streambuf&& rhs) :
    m_plogger(std::move(rhs.m_plogger)),
    m_log_msg(nullptr, rhs.m_log_msg.level)
{
    rhs.m_plogger = nullptr;
    if (m_plogger)
    {
        m_log_msg.logger_name = std::move(rhs.m_log_msg.logger_name);
        m_log_msg.time = std::move(rhs.m_log_msg.time);
        m_log_msg.thread_id = std::move(rhs.m_log_msg.thread_id);
        m_log_msg.raw = std::move(rhs.m_log_msg.raw);
        m_log_msg.formatted = std::move(rhs.m_log_msg.formatted);
    }
}

template <class CharT, class Traits>
spdlog::basic_streambuf<CharT, Traits>::~basic_streambuf()
{
    if (!m_plogger)
        return;

    try
    {
        m_plogger->_sink_it(m_log_msg);
    }
    catch (const std::exception &ex)
    {
        m_plogger->error_handler()(ex.what());
    }
    catch (...)
    {
        m_plogger->error_handler()("Unknown exception");
    }
}

template <class CharT, class Traits>
std::streamsize spdlog::basic_streambuf<CharT, Traits>::xsputn(const char_type* __s, std::streamsize __n)
{
    if (!m_plogger)
        return 0;

    try
    {
        m_log_msg.raw << fmt::BasicStringRef<char_type>(__s, __n);
    }
    catch (const std::exception &ex)
    {
        m_plogger->error_handler()(ex.what());
    }
    catch (...)
    {
        m_plogger->error_handler()("Unknown exception");
    }

    return __n;
}
