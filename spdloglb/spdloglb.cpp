// spdloglb.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//



#include "stdafx.h"
#include "spdlog/spdloglb_VarArg.h"
#include "spdlog/spdloglb_ostream.h"
#include "spdlog/cstyle/sink_handle.h"
#include "spdlog/cstyle/logger_handle.h"
#include "spdloglb_helpers.h"

//#include "Sink_mgr.h"
//#include "BaumEventLog_ResourceList.h"

namespace spdlog
{


	DWORD								g_InstanceCount = 0;


	void logger_error_handler(const std::string &err_msg);

	class spdloglib_error_manager
	{
	private:
		error_callback*		_callback;

	public:
		spdloglib_error_manager()
		{
			_callback = NULL;
		}

		void init(error_callback* lp_err_clb)
		{
			if (lp_err_clb)
				free();
			_callback = lp_err_clb;
			if (_callback)
			{
				spdlog::set_error_handler(logger_error_handler);
			}
		}
		void free(void)
		{
			spdlog::set_error_handler(nullptr);
		}

		void on_error(const char* lpMsg)
		{
			if (_callback)
				_callback->on_error(lpMsg);
		}

	}g_spdloglib_error_manager;

	
	void logger_error_handler(const std::string &err_msg)
	{
		g_spdloglib_error_manager.on_error(err_msg.c_str());
	}


	// Initialize the library and define some per instance modes
	// The library may get loaded and initialized by more than one modules of the current process. But the "per instance" values
	// has to be unique for one instance (one running exe). So this values are stored at the first call, the next call returns
	// BevlInitReturn_WasInitialized to indicate that this settings was not incorperated at all. So there are the following rules:
	//
	// * AsyncMode with all its settings (QueueSize ...) has to be defined unique to one application
	// * If any of the logs are accessed by more than one thread, it is required to set "MultiThreaded" to true.
	//   Missing this may cause really bad side effects (missing synchronisation)
	// * Never use the same logfile from two different applications (EXE-Files)
	// * Not really sure about the impact, but I think it is a good idea to write different logfiles for each instance,
	//	 if the program supports more than one instance.
	//
	// Parameters:
	// AsyncMode:				Speeds up logging by creating a queue for the log messages
	// QueuSize:				Size of the queue that gets created for each logger
	// OverflowPolicy:			Defines what to do if a queue is full.
	//							0 -> hold/sleep the program until the queue has space again
	//							1 -> discard the message
	// FlushIntervalMs:			Defines the interval to perform automatic flush operations on a queue
	SPDLOGLB_API spdlog::InitReturn SpdLog_Init(bool AsyncMode, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, error_callback* lp_err_clb)
	{
		if (g_InstanceCount++ == 0)
		{
			g_spdloglib_error_manager.init(lp_err_clb);

			spdlog::async_overflow_policy		opc = spdlog::async_overflow_policy::block_retry;

			switch (OverflowPolicy)
			{
			case unsigned long(spdlog::async_overflow_policy::block_retry) :
				opc = spdlog::async_overflow_policy::block_retry;
				break;

			case unsigned long(spdlog::async_overflow_policy::discard_log_msg) :
				opc = spdlog::async_overflow_policy::discard_log_msg;
				break;
			}

			std::chrono::milliseconds			fims = std::chrono::milliseconds::zero();
			fims += std::chrono::milliseconds(FlushIntervalMs);


			if (AsyncMode)
				spdlog::details::registry::instance().set_async_mode(QueueSize, opc, nullptr, fims, nullptr);
			else
				spdlog::details::registry::instance().set_sync_mode();

			return InitReturn_Succeed;
		}
		else
			return InitReturn_WasInitialized;
	}


	SPDLOGLB_API bool SpdLog_Free(void)
	{
		if (--g_InstanceCount == 0)
		{

			g_InstanceCount = 0;
			// Free all remaining objects
			sink_handle_factory::drop_all();
			logger_handle_factory::drop_all();
			spdlog::details::registry::instance().drop_all();
			spdlog::details::registry::instance().set_sync_mode();
			g_spdloglib_error_manager.free();
			return true;
		}
		return false;
	}



	// Ther is a rule for all filebased sinks: they should never point to the same file!
	// So all this functions look for a existing sink with the same name before a new one is created.
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateFileSink(const spdlog::filename_char_t* file_name, bool force_flush, bool MultiThreaded)
	{
		CBevlFnConverter fnc(file_name);
		
		spdlog::log_handle_t ret_val = NULL;
		try
		{
			ret_val = sink_handle_factory::create_simple_file_sink(fnc, force_flush, MultiThreaded);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}

		return ret_val;

	}


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateRotatingFileSink(const spdlog::filename_char_t* FileName, const spdlog::filename_char_t* FileExt, size_t max_file_size, size_t max_files,  bool MultiThreaded)
	{
		CBevlFnConverter fnc(FileName);
		CBevlFnConverter fnx(FileExt);

		spdlog::log_handle_t ret_val = NULL;
		try
		{
			ret_val = sink_handle_factory::create_rotating_file_sink(fnc, fnx,  MultiThreaded, max_file_size, max_files);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}

		return ret_val;
//		return sink_handle_factory::create_rotating_file_sink(fnc, fnx, force_flush, MultiThreaded, max_file_size, max_files);
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateDailyFileSink(const spdlog::filename_char_t* FileName, const spdlog::filename_char_t* FileExt, int rotation_hour, int rotation_minute, bool MultiThreaded)
	{
		CBevlFnConverter fnc(FileName);
		CBevlFnConverter fnx(FileExt);
		try
		{
			return sink_handle_factory::create_daily_file_sink(fnc, fnx, rotation_hour, rotation_minute,  MultiThreaded);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return NULL;
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateMsvcSink(bool MultiThreaded)
	{
		try
		{
			return sink_handle_factory::create_msvc_sink(MultiThreaded);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return NULL;
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStdoutSink(bool MultiThreaded, bool UseColor)
	{
		try
		{
			return sink_handle_factory::create_stdout_sink(MultiThreaded, UseColor);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return NULL;
//		return sink_handle_factory::create_stdout_sink(MultiThreaded, UseColor);
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStderrSink(bool MultiThreaded, bool UseColor)
	{
		try
		{
			return sink_handle_factory::create_stderr_sink(MultiThreaded, UseColor);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return NULL;
//		return sink_handle_factory::create_stderr_sink(MultiThreaded, UseColor);
	}


	// The sink handle is only required to make the calls to "SpdLog_CreateLogger". After you have created
	// your logger, you may call SpdLog_FreeSink to free your reference to the sink. Sinks taken by created loggers
	// get freed at the point when the logger is destroyed itself, completely behind your back. 
	SPDLOGLB_API bool SpdLog_FreeSink(spdlog::log_handle_t hSink)
	{
		return sink_handle_factory::free_sink(hSink);
	}


	// Creates a logger. 
	// Need 1-n output sinks that get created before with the appropriate functions
	// LoggerName has to be a unique name for each logger.
	// FormatString is the base format for each log entry, decorating it with timestamps ...
	// log_level is the current level from BevlLogLevel_trace to BevlLogLevel_off, all messages with lower LogLEvels are thrown away.
	// LogBitMask is the mask to filter out message classes in functions that uses a bit flag to indicate the message class
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateLogger(spdlog::log_handle_t* lpSinks, const unsigned long nSinks, const char* LoggerName, const spdlog::log_char_t* FormatString, unsigned long log_level, unsigned long LogBitMask)
	{
		try
		{
			logger_handle* lph = logger_handle_factory::create_logger_p(lpSinks, nSinks, LoggerName);
			if (lph)
			{
				lph->set_level(CBevlLogLevelConverter(log_level));
				lph->set_pattern(CBevlFormatstringConverter(FormatString));
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lph->set_enable_bit_mask(LogBitMask);
#endif
				return lph->GetHandle();
			}
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}

		return nullptr;

	}

	// Finds a existing logger. The logger list is valid for the complete process, so a logger may be used by more than one module at a time.
	// In this case it is best to query first for the logger using this function, because it is not possible to create two loggers
	// with the same name.
	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetLogger(const char* LoggerName)
	{
		return logger_handle_factory::get_logger(LoggerName);
	}

	SPDLOGLB_API bool SpdLog_DeleteLogger(spdlog::log_handle_t hLogger)
	{
		return logger_handle_factory::delete_logger(hLogger);
	}

	SPDLOGLB_API bool SpdLog_IsValidLoggerHandle(spdlog::log_handle_t hLogger)
	{
		return logger_handle_factory::validate_logger_handle(hLogger);
	}


	// Sets the basic log output pattern. It is a format string according to https://github.com/gabime/spdlog/wiki/3.-Custom-formatting.
	// Use this format string to do the base setup for the log output lines.
	// Attention: if called with hLogger == NULL, it resets the fomat pattern of all existing Loggers!
	SPDLOGLB_API bool SpdLog_SetLoggerFormatPattern(spdlog::log_handle_t hLogger, spdlog::fmt_formatchar_t* lpPattern)
	{
		return logger_handle_factory::set_logger_format_pattern(hLogger, lpPattern);
	}

	SPDLOGLB_API bool SpdLog_SetLoggerLevel(spdlog::log_handle_t hLogger, unsigned long log_level)
	{
		return logger_handle_factory::set_level(hLogger, CBevlLogLevelConverter(log_level));
	}

	SPDLOGLB_API bool SpdLog_SetLoggerBitMask(spdlog::log_handle_t hLogger, unsigned long BitMask)
	{

#ifdef SPDLOG_BITMASK_LOG_FILTER
		return logger_handle_factory::set_logger_enable_bitmask(hLogger, BitMask);
#else
		return false;
#endif
	}

	// Flushes the log if a message that matches the LogLEvel was loggt. 
	SPDLOGLB_API bool SpdLog_SetLoggerAutomaticFlush(spdlog::log_handle_t hLogger, unsigned long log_level)
	{
		return logger_handle_factory::set_logger_automatic_flush(hLogger, CBevlLogLevelConverter(log_level));
	}

	// Flushes the log file (to disk ...)
	SPDLOGLB_API bool SpdLog_LoggerFlush(spdlog::log_handle_t hLogger)
	{
		return logger_handle_factory::logger_flush(hLogger);
	}

	SPDLOGLB_API unsigned long SpdLog_GetLoggerLevel(spdlog::log_handle_t hLogger)
	{
		return logger_handle_factory::get_level(hLogger);
	}

	SPDLOGLB_API unsigned long SpdLog_GetLoggerBitMask(spdlog::log_handle_t hLogger)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER
		return logger_handle_factory::get_logger_enable_bitmask(hLogger);
#else
		return 0;
#endif

	}
	SPDLOGLB_API unsigned long SpdLog_GetLoggerAutomaticFlush(spdlog::log_handle_t hLogger)
	{
		return logger_handle_factory::get_logger_automatic_flush(hLogger);
	}


	SPDLOGLB_API bool SpdLog_GetLoggerName(spdlog::log_handle_t hLogger, char* lpBuffer, unsigned long BufSize)
	{
		if (lpBuffer == NULL)
			return false;
		if (BufSize == 0)
			return false;

		std::string s;
		if (logger_handle_factory::get_logger_name(hLogger, s))
		{
			unsigned long cpLen = s.length();
			if (cpLen >= BufSize)
				cpLen = BufSize - 1;
			strncpy_s(lpBuffer,BufSize, s.data(), cpLen);
			lpBuffer[cpLen] = '\0';
			return true;
		}
		return false;
	}



	SPDLOGLB_API bool SpdLog_SetLoggerErrorHandler(spdlog::log_handle_t hLogger, log_err_handler ErrorHandler)
	{
		return logger_handle_factory::set_error_handler(hLogger, ErrorHandler);
	}


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateSimpleFileLogger(const char* logger_name,
		const spdlog::filename_char_t* file_name,
		bool force_flush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		try
		{
			logger_handle* lpLh = logger_handle_factory::create_simple_file_logger_p(logger_name, file_name, force_flush, MultiThreaded);
			if (lpLh == nullptr)
				return nullptr;

			if (lp_prop)
			{
				if (lp_prop->FormatString)
					lpLh->set_pattern(lp_prop->FormatString);
				lpLh->set_level(lp_prop->Level);
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
			}
			return lpLh->GetHandle();
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}

		return nullptr;
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateSimpleFileLogger(const char* logger_name,
		const spdlog::filename_char_t* file_name,
		bool force_flush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{

		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateSimpleFileLogger(logger_name, file_name, force_flush, MultiThreaded, lp_prop);

	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateRotatingFileLogger(const char* logger_name,
		const spdlog::filename_char_t* file_name,
		const spdlog::filename_char_t* file_ext,
		size_t max_file_size,
		size_t max_files,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{

		try
		{
			logger_handle* lpLh = logger_handle_factory::create_rotating_file_logger_p(logger_name, file_name, file_ext, max_file_size, max_files, MultiThreaded);
			if (lpLh == nullptr)
				return nullptr;

			if (lp_prop)
			{
				lpLh->set_level(lp_prop->Level);
				if (lp_prop->FormatString)
					lpLh->set_pattern(lp_prop->FormatString);
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
			}
			return lpLh->GetHandle();
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}

	// Creates a rotating logfile restricted in file size and number of renamed files
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateOrGetRotatingFileLogger(const char* logger_name,
		const spdlog::filename_char_t* file_name,
		const spdlog::filename_char_t* file_ext,
		size_t max_file_size,
		size_t max_files,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateRotatingFileLogger(logger_name, file_name, file_ext, max_file_size, max_files, MultiThreaded, lp_prop);

	}

	// Creates a logfile that switch over to another file after a specified time.
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateDaylyFileLogger(const char* logger_name,
		const spdlog::filename_char_t* file_name,
		const spdlog::filename_char_t* file_ext,
		int rotation_hour,
		int rotation_minute,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		try
		{
			logger_handle* lpLh = logger_handle_factory::create_dayly_file_logger_p(logger_name, file_name, file_ext, rotation_hour, rotation_minute, MultiThreaded);
			if (lpLh == NULL)
				return nullptr;

			if (lp_prop)
			{
				lpLh->set_level(lp_prop->Level);
				if (lp_prop->FormatString)
					lpLh->set_pattern(lp_prop->FormatString);
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
			}
			return lpLh->GetHandle();
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateDaylyFileLogger(const char* logger_name,
		const spdlog::filename_char_t* file_name,
		const spdlog::filename_char_t* file_ext,
		int rotation_hour,
		int rotation_minute,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateDaylyFileLogger(logger_name, file_name, file_ext, rotation_hour, rotation_minute, MultiThreaded, lp_prop);

	}


	// Logger for Microsoft Visual Studio (OutputDebugString)
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateMsvcLogger(const char* logger_name,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{

		try
		{
			logger_handle* lpLh = logger_handle_factory::create_msvc_logger_p(logger_name, MultiThreaded);

			if (lpLh == NULL)
				return nullptr;

			if (lp_prop)
			{
				lpLh->set_level(lp_prop->Level);
				if (lp_prop->FormatString)
					lpLh->set_pattern(lp_prop->FormatString);
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
			}
			return lpLh->GetHandle();
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}


	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateMsvcLogger(const char* logger_name,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateMsvcLogger(logger_name, MultiThreaded, lp_prop);

	}


	// Logger mit Ausgabe nach stdout (console)
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStdoutLogger(const char* logger_name,
		bool use_color,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{

		try
		{
			logger_handle* lpLh = logger_handle_factory::create_stdout_logger_p(logger_name, use_color, MultiThreaded);

			if (lpLh == NULL)
				return nullptr;

			if (lp_prop)
			{
				lpLh->set_level(lp_prop->Level);
				if (lp_prop->FormatString)
					lpLh->set_pattern(lp_prop->FormatString);
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
			}
			return lpLh->GetHandle();
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateStdoutLogger(const char* logger_name,
		bool use_color,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{

		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateStdoutLogger(logger_name, use_color, MultiThreaded, lp_prop);

	}


	// Logger mit Ausgabe nach stderr (console)
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStderrLogger(const char* logger_name,
		bool use_color,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		try
		{
			logger_handle* lpLh = logger_handle_factory::create_stderr_logger_p(logger_name, use_color, MultiThreaded);

			if (lpLh == NULL)
				return nullptr;

			if (lp_prop)
			{
				lpLh->set_level(lp_prop->Level);
				if (lp_prop->FormatString)
					lpLh->set_pattern(lp_prop->FormatString);
#ifdef SPDLOG_BITMASK_LOG_FILTER
				lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
			}
			return lpLh->GetHandle();
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateStderrLogger(const char* logger_name,
		bool use_color,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{

		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateStderrLogger(logger_name, use_color, MultiThreaded, lp_prop);

	}



	// The log functions has the following common parameters:
	// log_level: is a value from BevlLogLevelEnum. If this value is lower than the loggers loglevel, the message is filtered out.
	// log_bit_flag: this value is compared to the mask from the logger and filtered out if the bit is not set.

	SPDLOGLB_API bool SpdLog_LogBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER
		// Direct cast to the enum
		return logger_handle_factory::log_bf(hLogger, spdlog::level::level_enum(log_level), log_bit_flag, log_string);

		return false;
#else
		return false;
#endif
	}

	SPDLOGLB_API bool SpdLog_LogBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER
		return logger_handle_factory::log_bfo(hLogger, log_bit_flag, log_string);

#else
		return false;
#endif
	}

	SPDLOGLB_API bool SpdLog_Log(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string)
	{
		return logger_handle_factory::log(hLogger, spdlog::level::level_enum(log_level), log_string);
	}




	// Formats the log output using printf format style. It uses the original printf functions, is not type safe and does
	// not use the format library of Victor Zvervovich, althogh it does supports printf style formating. To use this,
	// there are functions in "SpdLog_VarArg" that provide a way to handle variable parameter list in a typesafe way.
	
	
	SPDLOGLB_API bool SpdLog_LogPrintf(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string, ...)
	{
		va_list args;
		va_start(args, log_string);
		return SpdLog_vLogPrintf(hLogger, log_level, log_string, args);
	}

	SPDLOGLB_API bool SpdLog_vLogPrintf(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string, va_list args)
	{

		const unsigned long BUF_SIZE = 1024;

		spdlog::log_char_t Buffer[BUF_SIZE];

#ifdef SPDLOG_WCHAR_LOGGING
		_vsnwprintf_s(Buffer, BUF_SIZE, log_string, args);
#else
		_vsnprintf_s(Buffer, BUF_SIZE, log_string, args);
#endif
		return logger_handle_factory::log(hLogger, spdlog::level::level_enum(log_level), Buffer);
	}

	SPDLOGLB_API bool SpdLog_LogPrintfBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, ...)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER

		va_list args;
		va_start(args, log_string);
		return SpdLog_vLogPrintfBf(hLogger, log_level, log_bit_flag, log_string, args);

#else
		return false;
#endif // SPDLOG_BITMASK_LOG_FILTER

	}

	SPDLOGLB_API bool SpdLog_vLogPrintfBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, va_list args)
	{

#ifdef SPDLOG_BITMASK_LOG_FILTER
		const unsigned long BUF_SIZE = 1024;

		spdlog::log_char_t Buffer[BUF_SIZE+1];

#ifdef SPDLOG_WCHAR_LOGGING
		_vsnwprintf_s(Buffer, BUF_SIZE,_TRUNCATE, log_string, args);
#else
		_vsnprintf_s(Buffer, BUF_SIZE, _TRUNCATE, log_string, args);
#endif
		return logger_handle_factory::log_bf(hLogger, spdlog::level::level_enum(log_level), log_bit_flag, Buffer);
#else   // SPDLOG_BITMASK_LOG_FILTER
		return false;
#endif	// SPDLOG_BITMASK_LOG_FILTER
	}

	
	
	SPDLOGLB_API bool SpdLog_LogPrintfBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, ...)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER
		va_list args;
		va_start(args, log_string);
		return SpdLog_vLogPrintfBfo(hLogger, log_bit_flag, log_string, args);
#else
		return false;
#endif
	}

	SPDLOGLB_API bool SpdLog_vLogPrintfBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, va_list args)
	{

#ifdef SPDLOG_BITMASK_LOG_FILTER

		const unsigned long BUF_SIZE = 1024;
		spdlog::log_char_t Buffer[BUF_SIZE];

#ifdef SPDLOG_WCHAR_LOGGING
		_vsnwprintf_s(Buffer, BUF_SIZE, _TRUNCATE, log_string, args);
#else
		_vsnprintf_s(Buffer, BUF_SIZE, _TRUNCATE, log_string, args);
#endif
		return logger_handle_factory::log_bfo(hLogger, log_bit_flag, Buffer);
#else   // SPDLOG_BITMASK_LOG_FILTER
		return false;
#endif	// SPDLOG_BITMASK_LOG_FILTER

	}

	SPDLOGLB_API bool SpdLog_LogFormat(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string, vararg_list& args)
	{
		return logger_handle_factory::log_format(hLogger, spdlog::level::level_enum(log_level), log_string, args);
	}

	SPDLOGLB_API bool SpdLog_LogFormatBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t*  log_string, vararg_list& args)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER
		return logger_handle_factory::log_format_bf(hLogger, spdlog::level::level_enum(log_level), log_bit_flag, log_string, args);
#else
		return false;
#endif

	}

	SPDLOGLB_API bool SpdLog_LogFormatBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t*  log_string, vararg_list& args)
	{
#ifdef SPDLOG_BITMASK_LOG_FILTER
		return logger_handle_factory::log_format_bfo(hLogger, log_bit_flag, log_string, args);
#else
		return false;
#endif

	}


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateOstreamSink(spdlog::std_ostream_t& os, bool force_flush, bool MultiThreaded)
	{
		try
		{
			return spdlog::sink_handle_factory::create_ostream_sink(os, MultiThreaded, force_flush);
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateOstreamLogger(const char* logger_name,
		spdlog::std_ostream_t& os,
		bool force_flush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		try
		{
			spdlog::logger_handle* lpLh = logger_handle_factory::create_ostream_logger_p(logger_name, os, force_flush, MultiThreaded);
			if (lpLh)
			{
				if (lp_prop)
				{
					lpLh->set_pattern(lp_prop->FormatString);
					lpLh->set_level(lp_prop->Level);
#ifdef SPDLOG_BITMASK_LOG_FILTER
					lpLh->set_enable_bit_mask(lp_prop->LogBitMask);
#endif
				}
				return lpLh->GetHandle();
			}
			return nullptr;
		}
		catch (const std::exception &ex)
		{
			g_spdloglib_error_manager.on_error(ex.what());
		}
		catch (...)
		{
			g_spdloglib_error_manager.on_error("Unknown exception");
		}
		return nullptr;
	}


	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateOstreamLogger(const char* logger_name,
		spdlog::std_ostream_t& os,
		bool force_flush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop)
	{
		spdlog::log_handle_t hExt = logger_handle_factory::get_logger(logger_name);
		if (hExt != nullptr)
			return hExt;

		return SpdLog_CreateOstreamLogger(logger_name, os, force_flush, MultiThreaded, lp_prop);
	}



	// Have trouble to make it compile using the current buildin format library. Works with tester of the current format
	// library. Cancelled at the moment.
	//// The same way, just with the printf style format syntax of the format library.
	//// Attention: the format specifiers are not neccessarily the same that you know from "printf", look into the doku 
	//// of the format library. For example, "%S" for a string of the opposite character format (char/wchar) is not supported.
	//SPDLOGLB_API bool SpdLog_LogFormatPrintf(spdlog::log_handle_t hLogger, unsigned long log_level, spdlog::log_char_t* log_string, vararg_list& args);
	//SPDLOGLB_API bool SpdLog_LogFormatPrintfBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, spdlog::log_char_t* log_string, vararg_list& args);
	//SPDLOGLB_API bool SpdLog_LogFormatPrintfBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, spdlog::log_char_t* log_string, vararg_list& args);


} // namespace spdlog




