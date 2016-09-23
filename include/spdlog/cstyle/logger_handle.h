#pragma once
#include <memory>
#include <unordered_map>
#include <mutex>
#include "..\spdlog.h"
#include "spdlog_handle.h"
#include "vararg_list.h"
#include "sink_handle.h"


#define LGH_FAST_RAWPTR_ACCESS

namespace spdlog
{

	class format_args_converter
	{
	public:
		fmt::internal::Arg*	_merged_arguments;

		~format_args_converter()
		{
			if (_merged_arguments)
				delete[] _merged_arguments;
			_merged_arguments = NULL;
		}

		format_args_converter(fmt::internal::Value* lpVal, fmt::internal::Value::Type* lpType, size_t size)
		{
			_merged_arguments = NULL;
			if (size)
			{
				_merged_arguments = new fmt::internal::Arg[size];
				for (unsigned long i = 0; i < size; i++)
				{
//					fmt::internal::Arg* lpArg = &_merged_arguments[i];
					fmt::internal::Value* lpt = &_merged_arguments[i];
					*lpt = lpVal[i];
					_merged_arguments[i].type = lpType[i];
				}

			}
		}
	};

	// encapsules the resource stored in a shared pointer
	class logger_handle
	{
	private:
		// One shared ptr, should be in action only on creation and destruction: automatically free resource
		std::shared_ptr<spdlog::logger>		_Logger;

	public:
//		logger_handle() {}


		const std::string& name() const
		{
			return _Logger->name();
		}


		logger_handle(spdlog::sink_ptr& sink, const std::string& logger_name)
		{
			_Logger = spdlog::create(logger_name, sink);
		}

		logger_handle(spdlog::log_handle_t* lpSinks, const unsigned long nSinks, const std::string& LoggerName)
		{
			// Assort the required sinks

			std::vector<spdlog::sink_ptr> sinks;
			for (unsigned long i = 0; i < nSinks; i++)
			{
				sink_handle* lpSh = sink_handle_factory::get_sink(lpSinks[i]);
				if (lpSh)
				{
					sinks.push_back(lpSh->GetSinkSharedPointer());
				}
			}

			_Logger = spdlog::create(LoggerName, begin(sinks), end(sinks));

		}

		~logger_handle()
		{
			if (_Logger.get())
			{
				spdlog::drop(_Logger->name());
				_Logger.reset();
			}
		}
		// The raw logger pointer is also the handle itself, so access may be done in a 
		// fast way just by casting the handle to a pointer. 
		inline operator spdlog::logger*() const { return _Logger.get(); }
		inline operator spdlog::log_handle_t() const { return _Logger.get(); }
		inline spdlog::logger* GetHandle(void){ return _Logger.get(); }
		inline spdlog::logger* GetLoggerPointer(void) { return _Logger.get(); }

		inline void set_pattern(const spdlog::fmt_formatstring_t& format_string)
		{
			if (_Logger.get())
				_Logger->set_pattern(format_string);
		}
		inline void set_level(spdlog::level::level_enum level)
		{
			if (_Logger.get())
				_Logger->set_level(level);
		}

#ifdef SPDLOG_BITMASK_LOG_FILTER
		inline void set_enable_bit_mask(unsigned long bit_mask)
		{
			if (_Logger.get())
				_Logger->set_enable_bit_mask(bit_mask);
		}
#endif // SPDLOG_BITMASK_LOG_FILTER


	};




	class logger_handle_factory
	{
	private:
		using logger_handle_list_t = std::unordered_map<spdlog::log_handle_t, logger_handle*>;

		// Trick to generate static variables...
		static logger_handle_list_t & s_logger_handle_list() { static logger_handle_list_t  MyVal; return MyVal; }
		static std::mutex& s_lock_mutex() { static std::mutex MyVal; return MyVal; }


#ifdef LGH_FAST_RAWPTR_ACCESS
		inline static spdlog::logger* GetLogger(spdlog::log_handle_t hLogger) { return (spdlog::logger*)hLogger; }
#else
		inline static spdlog::logger* GetLogger(spdlog::log_handle_t hLogger)
		{
			// Need to lockup for list access...
			std::lock_guard<std::mutex> lock(s_lock_mutex());
			logger_handle_list_t::iterator it = s_sink_handle_list().find(hLogger);
			if (it != s_logger_handle_list().end())
			{
				return it->second->_Logger.get();
			}
			return nullptr;
		}
#endif



	public:

		// Cleanup all remaining objects
		static void drop_all(void)
		{
			logger_handle_list_t::iterator it;
			for (it = s_logger_handle_list().begin(); it != s_logger_handle_list().end(); it++)
			{
				if (it->second)
					delete it->second;
				it->second = NULL;
			}
			s_logger_handle_list().clear();
		}

		// "GetLogger" converts just the handle to a pointer, as long as the code is compiled with LGH_FAST_RAWPTR_ACCESS.
		// That is done to prevent the need to aquire the lock each time we have to access the logger.
		// It is very danger, so there is a function to validate a logger handle. But - it does a lock access, so 
		// do not integrate it in the standard logging workflow.
		static inline bool validate_logger_handle(spdlog::log_handle_t hLogger)
		{
			// Need to lockup for list access...
			std::lock_guard<std::mutex> lock(s_lock_mutex());
			logger_handle_list_t::iterator it = s_logger_handle_list().find(hLogger);
			if (it != s_logger_handle_list().end())
			{
				return true;
			}
			return false;
		}

		static inline std::string get_logger_name(spdlog::log_handle_t hLogger)
		{
			spdlog::logger* lpLogger = GetLogger(hLogger);
			if (lpLogger)
				return lpLogger->name();
			return "";
		}

		static inline bool get_logger_name(spdlog::log_handle_t hLogger, char* lpBuffer, unsigned long BufSize)
		{
			spdlog::logger* lpLogger = GetLogger(hLogger);
			if (lpLogger)
			{
				const std::string& sr = lpLogger->name();
				unsigned long cpLen = sr.length();
				if (cpLen >= BufSize)
					cpLen = BufSize;
				strncpy_s(lpBuffer,BufSize, sr.data(), cpLen);
				return true;
			}
			return false;
		}

		static bool set_error_handler(spdlog::log_handle_t hLogger, log_err_handler ErrorHandler)
		{
			spdlog::logger* lpLogger = GetLogger(hLogger);
			if (lpLogger)
			{
				lpLogger->set_error_handler(ErrorHandler);
				return true;
			}
			return false;
		}

		static inline bool log_format(spdlog::log_handle_t hLogger, spdlog::level::level_enum log_level, const spdlog::log_char_t* log_string, vararg_list& args)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				if(args.size() < fmt::ArgList::MAX_PACKED_ARGS)
				{
					fmt::internal::Value* lpVal = args.ValueArray();
					fmt::ArgList al(args.GetCompressedType(), lpVal);
					lpl->log(log_level, log_string, al);
				}
				else
				{
					format_args_converter fac(args.ValueArray(), args.ValueTypeArray(), args.size());
					fmt::ArgList al(args.GetCompressedType(), fac._merged_arguments);
					lpl->log(log_level, log_string, al);
				}
				//fmt::internal::Arg* lpData = args.data();
				//if (lpData)
				//{
				//	fmt::ArgList al(args.GetCompressedType(), lpData);
				//	lpl->log(log_level, log_string, al);
				//	return true;
				//}
			}
			return false;

		}

		static inline bool log(spdlog::log_handle_t hLogger, spdlog::level::level_enum log_level, const spdlog::log_char_t* log_string)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->log(log_level, log_string);
			}
			return true;
		}


#ifdef SPDLOG_BITMASK_LOG_FILTER
		static inline bool log_format_bf(spdlog::log_handle_t hLogger, spdlog::level::level_enum log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, vararg_list& args)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				if (args.size() < fmt::ArgList::MAX_PACKED_ARGS)
				{
					fmt::internal::Value* lpVal = args.ValueArray();
					fmt::ArgList al(args.GetCompressedType(), lpVal);
					lpl->log(log_level, log_bit_flag, log_string, al);
				}
				else
				{
					format_args_converter fac(args.ValueArray(), args.ValueTypeArray(), args.size());
					fmt::ArgList al(args.GetCompressedType(), fac._merged_arguments);
					lpl->log(log_level, log_bit_flag, log_string, al);
				}
				//fmt::internal::Arg* lpData = args.data();
				//if (lpData)
				//{
				//	fmt::ArgList al(args.GetCompressedType(), lpData);
				//	lpl->log(log_level, log_bit_flag, log_string, al);
				//	return true;
				//}
			}
			return true;
		}

		static inline bool log_format_bfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, vararg_list& args)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				if (args.size() < fmt::ArgList::MAX_PACKED_ARGS)
				{
					fmt::internal::Value* lpVal = args.ValueArray();
					fmt::ArgList al(args.GetCompressedType(), lpVal);
					lpl->log(log_bit_flag, log_string, al);
				}
				else
				{
					format_args_converter fac(args.ValueArray(), args.ValueTypeArray(), args.size());
					fmt::ArgList al(args.GetCompressedType(), fac._merged_arguments);
					lpl->log(log_bit_flag, log_string, al);
				}
				//fmt::internal::Arg* lpData = args.data();
				//if (lpData)
				//{
				//	fmt::ArgList al(args.GetCompressedType(), lpData);
				//	lpl->log(log_bit_flag, log_string, al);
				//	return true;
				//}
			}
			return true;
		}

		static inline bool log_bf(spdlog::log_handle_t hLogger, spdlog::level::level_enum log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->log(log_level, log_bit_flag, log_string);
			}
			return true;
		}

		static inline bool log_bfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->log( log_bit_flag, log_string);
			}
			return true;
		}
#endif



		static bool get_logger_name(spdlog::log_handle_t hLogger, std::string& name)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				name = lpl->name();
				return true;
			}
			return false;
		}

		static bool logger_flush(spdlog::log_handle_t hLogger)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->flush();
				return true;
			}


			return false;

		}

		static bool set_logger_automatic_flush(spdlog::log_handle_t hLogger, spdlog::level::level_enum log_level)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->flush_on(log_level);
				return true;
			}


			return false;

		}

		static spdlog::level::level_enum get_logger_automatic_flush(spdlog::log_handle_t hLogger)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				return lpl->get_flush_on();
			}
			return spdlog::level::off;

		}

#ifdef SPDLOG_BITMASK_LOG_FILTER
		static bool set_logger_enable_bitmask(spdlog::log_handle_t hLogger, unsigned long BitMask)
		{
			if (hLogger == nullptr)
			{
				spdlog::details::registry::instance().set_enable_bit_mask(BitMask);
				return true;
			}

			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->set_enable_bit_mask(BitMask);
				return true;
			}


			return false;
		}

		static unsigned long get_logger_enable_bitmask(spdlog::log_handle_t hLogger)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				return lpl->get_enable_bit_mask();
			}
			return 0;
		}
#endif


		static bool set_level(spdlog::log_handle_t hLogger, spdlog::level::level_enum level)
		{
			if (hLogger == nullptr)
			{
				spdlog::set_level(level);
				return true;
			}
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->set_level(level);
				return true;
			}


			return false;
		}

		static spdlog::level::level_enum get_level(spdlog::log_handle_t hLogger)
		{
			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				return lpl->get_level();
			}
			return spdlog::level::level_enum::off;
		}


		static bool set_logger_format_pattern(spdlog::log_handle_t hLogger, spdlog::fmt_formatstring_t FormatString)
		{
			if (hLogger == NULL)
			{
				spdlog::set_pattern(FormatString);
				return true;
			}

			spdlog::logger* lpl = GetLogger(hLogger);
			if (lpl)
			{
				lpl->set_pattern(FormatString);
				return true;
			}


			return false;

		}

		static bool delete_logger(spdlog::log_handle_t hLogger)
		{
			std::lock_guard<std::mutex> lock(s_lock_mutex());
			logger_handle_list_t::iterator it = s_logger_handle_list().find(hLogger);
			if (it != s_logger_handle_list().end())
			{
				std::string sLn;
				if (it->second)
				{
					delete it->second;
				}
				s_logger_handle_list().erase(it);
				return true;
			}
			return false;
		}

		static inline spdlog::log_handle_t get_logger(const std::string& Name)
		{
			std::shared_ptr<spdlog::logger> lpLog = spdlog::get(Name);
			// The handle is the raw pointer to the logger itself.
			return lpLog.get();

		}


		static inline logger_handle* create_logger_p(spdlog::log_handle_t* lp_sinks, const unsigned long n_sinks, const std::string& logger_name)
		{
			std::lock_guard<std::mutex> lock(s_lock_mutex());
			logger_handle* lh = new logger_handle(lp_sinks, n_sinks, logger_name);
			s_logger_handle_list().insert(logger_handle_list_t::value_type(lh->GetHandle(), lh));
			return lh;
		}

		static inline logger_handle* create_logger_p(spdlog::sink_ptr& sink, const std::string& logger_name)
		{
			std::lock_guard<std::mutex> lock(s_lock_mutex());
			logger_handle* lh = new logger_handle(sink, logger_name);
			s_logger_handle_list().insert(logger_handle_list_t::value_type(lh->GetHandle(), lh));
			return lh;
		}

		static inline spdlog::log_handle_t create_logger(spdlog::log_handle_t* lp_sinks, const unsigned long n_sinks, const std::string& logger_name)
		{
			return create_logger_p(lp_sinks, n_sinks, logger_name)->GetHandle();
		}

		static inline spdlog::log_handle_t create_logger_ex_plb(spdlog::log_handle_t* lp_sinks,
			const unsigned long n_sinks,
			const std::string& logger_name,
			const fmt_formatstring_t& pattern,
			spdlog::level::level_enum level,
			unsigned long bit_flag
		)
		{
			logger_handle* lpl = create_logger_p(lp_sinks, n_sinks, logger_name);
			if (lpl)
			{
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpl->set_enable_bit_mask(bit_flag);
#endif
				lpl->set_level(level);
				lpl->set_pattern(pattern);
			}
			return lpl->GetHandle();
		}

		static inline spdlog::log_handle_t create_logger(spdlog::sink_ptr& sink, const std::string& logger_name)
		{
			return create_logger_p(sink, logger_name)->GetHandle();
		}




		static inline logger_handle* create_simple_file_logger_p(std::string logger_name,
			spdlog::filename_t file_name,
			bool force_flush,
			bool multithreaded)
		{
			spdlog::sink_ptr ps;
			if (multithreaded)
				ps = std::make_shared<spdlog::sinks::simple_file_sink_mt>(file_name, force_flush);
			else
				ps = std::make_shared<spdlog::sinks::simple_file_sink_st>(file_name, force_flush);

			if (ps.get() == NULL)
				return nullptr;

			return create_logger_p(ps, logger_name);
		}

		static inline spdlog::log_handle_t create_simple_file_logger(std::string logger_name,
			spdlog::filename_t file_name,
			bool force_flush,
			bool multithreaded)
		{
			return create_simple_file_logger_p(logger_name, file_name, force_flush, multithreaded);
		}

		static inline logger_handle* create_rotating_file_logger_p(std::string logger_name,
			spdlog::filename_t file_name,
			spdlog::filename_t file_ext,
			size_t max_file_size,
			size_t max_files,
			bool MultiThreaded)
		{
			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_name, file_ext, max_file_size, max_files);
			else
				ps = std::make_shared<spdlog::sinks::rotating_file_sink_st>(file_name, file_ext, max_file_size, max_files);

			return create_logger_p(ps, logger_name);

		}


		static inline spdlog::log_handle_t  create_rotating_file_logger(std::string logger_name,
			spdlog::filename_t file_name,
			spdlog::filename_t file_ext,
			size_t max_file_size,
			size_t max_files,
			bool MultiThreaded)
		{
			return create_rotating_file_logger_p(logger_name, file_name, file_ext, max_file_size, max_files,  MultiThreaded);
		}

		static inline logger_handle* create_dayly_file_logger_p(std::string logger_name,
			spdlog::filename_t file_name,
			spdlog::filename_t file_ext,
			int rotation_hour,
			int rotation_minute,
			bool MultiThreaded)
		{
			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::daily_file_sink_mt>(file_name, file_ext, rotation_hour, rotation_minute);
			else
				ps = std::make_shared<spdlog::sinks::daily_file_sink_st>(file_name, file_ext, rotation_hour, rotation_minute);

			return create_logger_p(ps, logger_name);
		}

		static inline spdlog::log_handle_t create_dayly_file_logger(std::string logger_name,
			spdlog::filename_t file_name,
			spdlog::filename_t file_ext,
			int rotation_hour,
			int rotation_minute,
			bool force_flush,
			bool MultiThreaded)
		{
			return create_dayly_file_logger_p(logger_name, file_name, file_ext, rotation_hour, rotation_minute, MultiThreaded)->GetHandle();
		}

		static inline logger_handle*  create_msvc_logger_p(std::string logger_name,
			bool MultiThreaded)
		{
			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::msvc_sink_mt>();
			else
				ps = std::make_shared<spdlog::sinks::msvc_sink_st>();

			return create_logger_p(ps, logger_name);
		}

		static inline spdlog::log_handle_t create_msvc_logger(std::string logger_name,
			bool MultiThreaded)
		{
			return create_msvc_logger_p(logger_name, MultiThreaded)->GetHandle();
		}

		static inline logger_handle*  create_stdout_logger_p(std::string logger_name,
			bool use_color,
			bool MultiThreaded)
		{
			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = spdlog::sinks::stdout_sink_mt::instance();
			else
				ps = spdlog::sinks::stdout_sink_st::instance();

			if (use_color) //use color wrapper sink
				ps = std::make_shared<spdlog::sinks::ansicolor_sink>(ps);

			return create_logger_p(ps, logger_name);

		}

		static inline spdlog::log_handle_t create_stdout_logger(std::string logger_name,
			bool use_color,
			bool MultiThreaded)
		{
			return create_stdout_logger_p(logger_name, use_color, MultiThreaded)->GetHandle();
		}

		static inline logger_handle*  create_stderr_logger_p(std::string logger_name,
			bool use_color,
			bool MultiThreaded)
		{
			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = spdlog::sinks::stderr_sink_mt::instance();
			else
				ps = spdlog::sinks::stderr_sink_st::instance();

			if (use_color) //use color wrapper sink
				ps = std::make_shared<spdlog::sinks::ansicolor_sink>(ps);

			return create_logger_p(ps, logger_name);

		}

		static inline spdlog::log_handle_t create_stderr_logger(std::string logger_name,
			bool use_color,
			bool MultiThreaded)
		{
			return create_stderr_logger_p(logger_name, use_color, MultiThreaded)->GetHandle();
		}


		static inline logger_handle*  create_ostream_logger_p(std::string logger_name,
			spdlog::std_ostream_t& os,
			bool force_flush,
			bool MultiThreaded)
		{
			spdlog::sink_ptr ps;

			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::ostream_sink_mt>(os, force_flush);
			else
				ps = std::make_shared<spdlog::sinks::ostream_sink_st>(os, force_flush);

			return create_logger_p(ps, logger_name);
		}

		static inline logger_handle*  create_ostream_logger(std::string logger_name,
			spdlog::std_ostream_t& os,
			bool force_flush,
			bool MultiThreaded)
		{
			return create_ostream_logger_p(logger_name, os, force_flush, MultiThreaded);
		}

	};

}
