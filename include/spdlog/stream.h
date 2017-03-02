//
// Copyright(c) 2017 Benoit Leforestier.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <ostream>

namespace spdlog
{
    class logger;

    template <class CharT, class Traits = std::char_traits<CharT>>
    class basic_streambuf : public std::basic_streambuf<CharT, Traits>
    {
    public:
        typedef std::basic_streambuf<CharT, Traits> _Mysb;
        typedef CharT char_type;
        typedef Traits traits_type;
        typedef typename Traits::int_type int_type;
        typedef typename Traits::pos_type pos_type;
        typedef typename Traits::off_type off_type;

        basic_streambuf(logger* plogger, level::level_enum lvl);
        basic_streambuf(basic_streambuf&& rhs);
        virtual ~basic_streambuf();

        basic_streambuf(const basic_streambuf&) = delete;
        basic_streambuf& operator=(const basic_streambuf&) = delete;
        basic_streambuf& operator=(basic_streambuf&&) = delete;

    protected:
        virtual std::streamsize xsputn(const char_type* __s, std::streamsize __n) override;

    private:
        logger* m_plogger;
        details::log_msg m_log_msg;
    };

    template <class CharT, class Traits = std::char_traits<CharT>>
    class basic_ostream : public std::basic_ostream < CharT, Traits >
    {
    public:
        typedef std::basic_ostream<CharT, Traits> _Mybase;
        typedef CharT char_type;
        typedef Traits traits_type;
        typedef typename Traits::int_type int_type;
        typedef typename Traits::pos_type pos_type;
        typedef typename Traits::off_type off_type;

        basic_ostream() : _Mybase(&m_rdbuf), m_rdbuf(nullptr, level::critical) {}
        basic_ostream(spdlog::logger* plogger, level::level_enum lvl) : _Mybase(&m_rdbuf), m_rdbuf(plogger, lvl) {}
        basic_ostream(basic_ostream&& rhs_stream) : _Mybase(&m_rdbuf), m_rdbuf(std::move(rhs_stream.m_rdbuf)) {}
        virtual ~basic_ostream() = default;

        basic_ostream(const basic_ostream&) = delete;
        basic_ostream& operator=(const basic_ostream&) = delete;

        basic_ostream& operator=(basic_ostream&& rhs_stream) { swap(rhs_stream); return *this; }

        basic_streambuf<char_type>* rdbuf() const
        {
            basic_streambuf<char_type>* pBuf = _Mybase::rdbuf();
            if (!pBuf)
                return const_cast<basic_streambuf<char_type>*>(&m_rdbuf);

            return reinterpret_cast<basic_streambuf<char_type>*>(pBuf);
        }

    private:
        basic_streambuf<char_type> m_rdbuf;
    };

    typedef basic_streambuf<char> streambuf;
    typedef basic_streambuf<wchar_t> wstreambuf;

    typedef basic_ostream<char> ostream;
    typedef basic_ostream<wchar_t> wostream;
}
