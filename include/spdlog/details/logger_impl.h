//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <memory>
#include <string>


// create logger with given name, sinks and the default pattern formatter
// all other ctors will call this one
template<class It>
inline spdlog::logger::logger(const std::string& logger_name, const It& begin, const It& end):
    _name(logger_name),
    _sinks(begin, end),
    _formatter(std::make_shared<pattern_formatter>(_SFS("%+")))
{
    _level = level::info;
    _flush_level = level::off;
    _last_err_time = 0;
	
	#ifdef SPDLOG_BITMASK_LOG_FILTER
	_log_enable_bit_mask = 0;
	#endif
    _err_handler = [this](const std::string &msg)
    {
        this->_default_err_handler(msg);
    };
}

// ctor with sinks as init list
inline spdlog::logger::logger(const std::string& logger_name, sinks_init_list sinks_list):
    logger(logger_name, sinks_list.begin(), sinks_list.end())
{}


// ctor with single sink
inline spdlog::logger::logger(const std::string& logger_name, spdlog::sink_ptr single_sink):
    logger(logger_name,
{
    single_sink
})
{}


inline spdlog::logger::~logger() = default;


inline void spdlog::logger::set_formatter(spdlog::formatter_ptr msg_formatter)
{
    _set_formatter(msg_formatter);
}

inline void spdlog::logger::set_pattern(const fmt_formatstring_t& pattern)
{
    _set_pattern(pattern);
}


template <typename... Args>
inline void spdlog::logger::log(level::level_enum lvl, const fmt_formatchar_t* fmt, const Args&... args)
{
    if (!should_log(lvl)) return;

    try
    {
        details::log_msg log_msg(&_name, lvl);
        log_msg.raw.write(fmt, args...);
        _sink_it(log_msg);
    }
    catch (const std::exception &ex)
    {
        _err_handler(ex.what());
    }
    catch (...)
    {
        _err_handler("Unknown exception");
    }
}

template <typename... Args>
inline void spdlog::logger::log(level::level_enum lvl, const log_char_t* msg)
{
    if (!should_log(lvl)) return;
    try
    {
        details::log_msg log_msg(&_name, lvl);
        log_msg.raw << msg;
        _sink_it(log_msg);
    }
    catch (const std::exception &ex)
    {
        _err_handler(ex.what());
    }
    catch (...)
    {
        _err_handler("Unknown exception");
    }

}

template<typename T>
inline void spdlog::logger::log(level::level_enum lvl, const T& msg)
{
    if (!should_log(lvl)) return;
    try
    {
        details::log_msg log_msg(&_name, lvl);
        log_msg.raw << msg;
        _sink_it(log_msg);
    }
    catch (const std::exception &ex)
    {
        _err_handler(ex.what());
    }
    catch (...)
    {
        _err_handler("Unknown exception");
    }
}


template <typename... Args>
inline void spdlog::logger::trace(const fmt_formatchar_t* fmt, const Args&... args)
{
    log(level::trace, fmt, args...);
}

template <typename... Args>
inline void spdlog::logger::debug(const fmt_formatchar_t* fmt, const Args&... args)
{
    log(level::debug, fmt, args...);
}

template <typename... Args>
inline void spdlog::logger::info(const fmt_formatchar_t* fmt, const Args&... args)
{
    log(level::info, fmt, args...);
}


template <typename... Args>
inline void spdlog::logger::warn(const fmt_formatchar_t* fmt, const Args&... args)
{
    log(level::warn, fmt, args...);
}

template <typename... Args>
inline void spdlog::logger::error(const fmt_formatchar_t* fmt, const Args&... args)
{
    log(level::err, fmt, args...);
}

template <typename... Args>
inline void spdlog::logger::critical(const fmt_formatchar_t* fmt, const Args&... args)
{
    log(level::critical, fmt, args...);
}


template<typename T>
inline void spdlog::logger::trace(const T& msg)
{
    log(level::trace, msg);
}

template<typename T>
inline void spdlog::logger::debug(const T& msg)
{
    log(level::debug, msg);
}


template<typename T>
inline void spdlog::logger::info(const T& msg)
{
    log(level::info, msg);
}


template<typename T>
inline void spdlog::logger::warn(const T& msg)
{
    log(level::warn, msg);
}

template<typename T>
inline void spdlog::logger::error(const T& msg)
{
    log(level::err, msg);
}

template<typename T>
inline void spdlog::logger::critical(const T& msg)
{
    log(level::critical, msg);
}




//
// name and level
//
inline const std::string& spdlog::logger::name() const
{
    return _name;
}

inline void spdlog::logger::set_level(spdlog::level::level_enum log_level)
{
    _level.store(log_level);
}

inline spdlog::level::level_enum spdlog::logger::get_level(void) const
{
	return (spdlog::level::level_enum)_level.load();
}


inline void spdlog::logger::set_error_handler(spdlog::log_err_handler err_handler)
{
    _err_handler = err_handler;
}

inline spdlog::log_err_handler spdlog::logger::error_handler()
{
    return _err_handler;
}


inline void spdlog::logger::flush_on(level::level_enum log_level)
{
    _flush_level.store(log_level);
}

inline spdlog::level::level_enum spdlog::logger::get_flush_on(void) const
{
	return (spdlog::level::level_enum)_flush_level.load();
}


inline spdlog::level::level_enum spdlog::logger::level() const
{
    return static_cast<spdlog::level::level_enum>(_level.load(std::memory_order_relaxed));
}

inline bool spdlog::logger::should_log(spdlog::level::level_enum msg_level) const
{
    return msg_level >= _level.load(std::memory_order_relaxed);
}

//
// protected virtual called at end of each user log call (if enabled) by the line_logger
//
inline void spdlog::logger::_sink_it(details::log_msg& msg)
{
    _formatter->format(msg);
    for (auto &sink : _sinks)
    {
        if( sink->should_log( msg.level))
        {
            sink->log(msg);
        }
    }

    if(_should_flush_on(msg))
        flush();
}

inline void spdlog::logger::_set_pattern(const fmt_formatstring_t& pattern)
{
    _formatter = std::make_shared<pattern_formatter>(pattern);
}
inline void spdlog::logger::_set_formatter(formatter_ptr msg_formatter)
{
    _formatter = msg_formatter;
}

inline void spdlog::logger::flush()
{
    for (auto& sink : _sinks)
        sink->flush();
}

inline void spdlog::logger::_default_err_handler(const std::string &msg)
{
    auto now = time(nullptr);
    if (now - _last_err_time < 60)
        return;
    auto tm_time = details::os::localtime(now);
    char date_buf[100];
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
    details::log_msg  err_msg;
    err_msg.formatted.write(_SFS("[*** LOG ERROR ***] [{}] [{}] [{}]{}"), name(), msg, date_buf, details::os::eol);
    sinks::stderr_sink_mt::instance()->log(err_msg);
    _last_err_time = now;
}


#ifdef SPDLOG_BITMASK_LOG_FILTER
inline void spdlog::logger::set_enable_bit_mask(unsigned long mask)
{
	_log_enable_bit_mask = mask;
}

inline unsigned long spdlog::logger::get_enable_bit_mask()
{
	return _log_enable_bit_mask;
}

inline bool spdlog::logger::should_log_b(unsigned long BitFlag)
{
	return BitFlag & _log_enable_bit_mask;
}

inline bool spdlog::logger::should_log_bl(unsigned long BitFlag, level::level_enum level)
{
	if (level >= _level.load(std::memory_order_relaxed))
		return BitFlag & _log_enable_bit_mask;
	return false;
}

//template <typename... Args>
//void spdlog::logger::printf(level::level_enum lvl, unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList)
//{
//	if (!should_log_bl(lvl,BitFlag)) return;
//	try
//	{
//		details::log_msg log_msg(&_name, lvl);
//		log_msg.raw << fmt::sprintf(fmt, argList);
//		_sink_it(log_msg);
//	}
//	catch (const std::exception &ex)
//	{
//		_err_handler(ex.what());
//	}
//	catch (...)
//	{
//		_err_handler("Unknown exception");
//	}
//
//}
//
//template <typename... Args>
//void spdlog::logger::printf(unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList)
//{
//	if (!should_log_b(BitFlag)) return;
//	try
//	{
//		details::log_msg log_msg(&_name, lvl);
//		log_msg.raw << fmt::sprintf(fmt, argList);
//		_sink_it(log_msg);
//	}
//	catch (const std::exception &ex)
//	{
//		_err_handler(ex.what());
//	}
//	catch (...)
//	{
//		_err_handler("Unknown exception");
//	}
//
//
//}


template <typename... Args>
inline void spdlog::logger::log(level::level_enum lvl, const fmt_formatchar_t* fmt, fmt::ArgList& argList)
{
	if (!should_log(lvl)) return;

	try
	{
		details::log_msg log_msg(&_name, lvl);
		log_msg.raw.write(fmt, argList);
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}


template <typename... Args>
inline void spdlog::logger::log(level::level_enum lvl, unsigned long BitFlag, const fmt_formatchar_t* fmt, const Args&... args)
{
	if (!should_log_bl(BitFlag,lvl)) return;

	try
	{
		details::log_msg log_msg(&_name, lvl);
		log_msg.raw.write(fmt, args...);
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}

template <typename... Args>
inline void spdlog::logger::log(level::level_enum lvl, unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList)
{
	if (!should_log_bl(BitFlag, lvl)) return;

	try
	{
		details::log_msg log_msg(&_name, lvl);
		log_msg.raw.write(fmt, argList);
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}

template <typename... Args>
inline void spdlog::logger::log(level::level_enum lvl, unsigned long BitFlag, const log_char_t* msg)
{
	if (!should_log_bl(BitFlag,lvl)) return;
	try
	{
		details::log_msg log_msg(&_name, lvl);
		log_msg.raw << msg;
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}

}

template<typename T>
inline void spdlog::logger::log(level::level_enum lvl, unsigned long BitFlag, const T& msg)
{
	if (!should_log_bl(BitFlag,lvl)) return;
	try
	{
		details::log_msg log_msg(&_name, lvl);
		log_msg.raw << msg;
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}



template <typename... Args>
inline void spdlog::logger::log(unsigned long BitFlag, const fmt_formatchar_t* fmt, const Args&... args)
{
	if (!should_log_b(BitFlag)) return;

	try
	{
		details::log_msg log_msg(&_name, lvl);
		log_msg.raw.write(fmt, args...);
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}

template <typename... Args>
inline void spdlog::logger::log(unsigned long BitFlag, const fmt_formatchar_t* fmt, fmt::ArgList& argList)
{
	if (!should_log_b(BitFlag)) return;

	try
	{
		details::log_msg log_msg(&_name, level::level_enum::off);
		log_msg.raw.write(fmt, argList);
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}


template <typename... Args>
inline void spdlog::logger::log(unsigned long BitFlag, const log_char_t* msg)
{
	if (!should_log_b(BitFlag)) return;
	try
	{
		details::log_msg log_msg(&_name, level::level_enum::off);
		log_msg.raw << msg;
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}

}

template<typename T>
inline void spdlog::logger::log(unsigned long BitFlag, const T& msg)
{
	if (!should_log_b(BitFlag)) return;
	try
	{
		details::log_msg log_msg(&_name, level::level_enum::off);
		log_msg.raw << msg;
		_sink_it(log_msg);
	}
	catch (const std::exception &ex)
	{
		_err_handler(ex.what());
	}
	catch (...)
	{
		_err_handler("Unknown exception");
	}
}
#endif

inline bool spdlog::logger::_should_flush_on(const details::log_msg &msg)
{
    const auto flush_level = _flush_level.load(std::memory_order_relaxed);
    return (msg.level >= flush_level) && (msg.level != level::off);
}
