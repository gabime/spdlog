#pragma once

// Source stand: abgeglichen mit Master_20160906

#include "..\include\spdlog\common_types.h"



#ifdef SPDLOGLB_EXPORTS
#define SPDLOGLB_API __declspec(dllexport)
#else
#define SPDLOGLB_API __declspec(dllimport)
#endif

#define BEVL_OSTREAM_SUPPORT


namespace spdlog
{

	typedef enum
	{
		InitReturn_Failled = 0,
		InitReturn_Succeed = 1,
		InitReturn_WasInitialized = 2,
	}InitReturn;


	typedef struct SPDLOGLB_CREATELOGGER_PARAM
	{
		spdlog::fmt_formatchar_t*		FormatString;
		spdlog::level::level_enum		Level;
		// BitMask is only used if library was compiled with SPDLOG_BITMASK_LOG_FILTER define!
		unsigned long					LogBitMask;
	}SPDLOGLB_CREATELOGGER_PARAM;

	class error_callback
	{
	public:
		virtual void on_error(const char* lp_msg) {}
	};

#ifdef __cplusplus
extern "C" {
#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	//
	// AsyncMode:				Speeds up logging by creating a queue for the log messages
	//
	// QueuSize:				Size of the queue that gets created for each logger
	//
	// OverflowPolicy:			Defines what to do if a queue is full.
	//							0 -> hold/sleep the program until the queue has space again
	//							1 -> discard the message
	//
	// FlushIntervalMs:			Defines the interval to perform automatic flush operations on a queue
	//
	// lp_err_clb:				Callback is utilized if a error exception is thrown inside of the library.
	//							This parameter is also application global and is not applied if the function returns
	//							BevlInitReturn_WasInitialized !
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SPDLOGLB_API spdlog::InitReturn SpdLog_Init(bool AsyncMode, unsigned long QueueSize, unsigned long OverflowPolicy, unsigned long FlushIntervalMs, error_callback* lp_err_clb);

	// Free the library according to its InstanceCounter. Returns true, if the library was really removed
	// because its instance counter has reached 0.
	SPDLOGLB_API bool SpdLog_Free(void);


	///////////////////////////////////////////////////////////////////////////////
	// Simple functions to create loggers or get existing loggers. Each logger has
	// got a single output sink, multiple sinks are not supported. For these special
	// cases you need to build your logger using the construction kit below where 
	// you can create sinks and use them to initialize loggers. 
	//
	// Common Parameters:
	//	FormatString:	Base format of each log entry (decoration, timestamp ...)
	//	log_level:		the current level from BevlLogLevel_trace to BevlLogLevel_off, 
	//					all messages with lower LogLEvels are thrown away.		
	//	LogBitMask:		mask to filter out message classes in log functions that uses 
	//					a bit flag to indicate the message classes to pass through
	//	MultiThreaded:	Set this to true if you feed the logger from more than one thread
	//	force_flush:	Forces a flush after a log entry is written (file, stream...)
	//	UseColor:		Possibility to use color formatting in console based sinks
	//
	// Create...Logger functions create a logger and faills if the logger (name, sink) exists.
	// GetOrCreate...Logger functions retrieves a existing logger if the logger exists.
	// Get...Logger functions retrieve the existing logger where name and target matches.
	///////////////////////////////////////////////////////////////////////////////


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateSimpleFileLogger(const char* LoggerName,
		const spdlog::filename_char_t* FileName,
		bool force_flush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateSimpleFileLogger(const char* LoggerName,
		const spdlog::filename_char_t* FileName,
		bool force_flush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateRotatingFileLogger(const char* LoggerName,
		const spdlog::filename_char_t* FileName,
		const spdlog::filename_char_t* FileExt,
		size_t max_file_size,
		size_t max_files,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	// Creates a rotating logfile restricted in file size and number of renamed files
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateOrGetRotatingFileLogger(const char* LoggerName,
		const spdlog::filename_char_t* FileName,
		const spdlog::filename_char_t* FileExt,
		size_t max_file_size,
		size_t max_files,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	// Creates a logfile that switch over to another file after a specified time.
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateDaylyFileLogger(const char* LoggerName,
		const spdlog::filename_char_t* FileName,
		const spdlog::filename_char_t* FileExt,
		int rotation_hour,
		int rotation_minute,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateDaylyFileLogger(const char* LoggerName,
		const spdlog::filename_char_t* FileName,
		const spdlog::filename_char_t* FileExt,
		int rotation_hour,
		int rotation_minute,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);



	// Logger for Microsoft Visual Studio (OutputDebugString)
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateMsvcLogger(const char* LoggerName,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateMsvcLogger(const char* LoggerName,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);



	// Logger mit Ausgabe nach stdout (console)
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStdoutLogger(const char* LoggerName,
		bool UseColor,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateStdoutLogger(const char* LoggerName,
		bool UseColor,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);


	// Logger mit Ausgabe nach stderr (console)
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStderrLogger(const char* LoggerName,
		bool UseColor,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateStderrLogger(const char* LoggerName,
		bool UseColor,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);



	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create sinks of different coleur to log data. A sink represents a output media like a file or a console...
	// To create a logger, at least one sink is required.
	// 
	// The parameter multithreaded is important: doing it wrong can cause various errors or performance loss.
	// If you are sure, that one sink is not used by more than one thread, you may set it to false to win a performance boost.
	// If one sink is used by more than one thread, you have to set it to true or your program will crash. 
	//
	// If you use the logger from different modules, you have to know that it is not possible to create a sink with the same
	// filename or type twice in the same process. But it is possible to use the sink for different loggers. 
	// So, if the sink allready exists, the creation function would fail (return NULL). Then you have to use the 
	// Get...Sink-functions to retreive the existing sink. 
	// A exclusion is the Ostream-Sink. You may create as much OStream-Sinks as you like.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateFileSink(const spdlog::filename_char_t* FileName, bool force_flush, bool MultiThreaded);
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateRotatingFileSink(const spdlog::filename_char_t* FileName, const spdlog::filename_char_t* FileExt, size_t max_file_size, size_t max_files, bool force_flush, bool MultiThreaded);
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateDailyFileSink(const spdlog::filename_char_t* FileName, const spdlog::filename_char_t* FileExt, int rotation_hour, int rotation_minute,  bool MultiThreaded);


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateMsvcSink(bool MultiThreaded);
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStdoutSink(bool MultiThreaded, bool UseColor);
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateStderrSink(bool MultiThreaded, bool UseColor);


	// The sink handle is only required to make the calls to "SpdLog_CreateLogger". After you have created
	// your logger, you may call SpdLog_FreeSink to free your reference to the sink. Sinks taken by created loggers
	// get freed at the point when the logger is destroyed itself, completely behind your back. 
	SPDLOGLB_API bool SpdLog_FreeSink(spdlog::log_handle_t hSink);


	// Creates a logger. 
	// Need 1-n output sinks that get created before with the appropriate functions
	// LoggerName has to be a unique name for each logger.
	// FormatString is the base format for each log entry, decorating it with timestamps ...
	// log_level is the current level from BevlLogLevel_trace to BevlLogLevel_off, all messages with lower LogLEvels are thrown away.
	// LogBitMask is the mask to filter out message classes in functions that uses a bit flag to indicate the message class
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateLogger(spdlog::log_handle_t* lpSinks, const unsigned long nSinks, const char* LoggerName, const spdlog::log_char_t* FormatString, unsigned long log_level, unsigned long LogBitMask);

	// Finds a existing logger. The logger list is valid for the complete process, so a logger may be used by more than one module at a time.
	// In this case it is best to query first for the logger using this function, because it is not possible to create two loggers
	// with the same name.
	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetLogger(const char* LoggerName);

	SPDLOGLB_API bool SpdLog_DeleteLogger(spdlog::log_handle_t hLogger);

	// Check a handle if it points to a valid logger.
	// Most functions that takes a logger handle does access the logger regardless of its state. If a 
	// logger is deleted, the code will fail in a ugly way. So this function can be used to check whether a 
	// handle is pointing to a existing logger or not. 
	SPDLOGLB_API bool SpdLog_IsValidLoggerHandle(spdlog::log_handle_t hLogger);


	// Sets the basic log output pattern. It is a format string according to https://github.com/gabime/spdlog/wiki/3.-Custom-formatting.
	// Use this format string to do the base setup for the log output lines.
	// Attention: if called with hLogger == NULL, it resets the fomat pattern of all existing Loggers!
	SPDLOGLB_API bool SpdLog_SetLoggerFormatPattern(spdlog::log_handle_t hLogger, spdlog::fmt_formatchar_t* lpPattern);

	// If called wich hLogger == nullptr, the logger level of all existing loggers are altered
	SPDLOGLB_API bool SpdLog_SetLoggerLevel(spdlog::log_handle_t hLogger, unsigned long log_level);

	// If called wich hLogger == nullptr, the bitmask of all existing loggers are altered
	SPDLOGLB_API bool SpdLog_SetLoggerBitMask(spdlog::log_handle_t hLogger, unsigned long BitMask);

	// Flushes the log if a message that matches the LogLEvel was loggt. 
	SPDLOGLB_API bool SpdLog_SetLoggerAutomaticFlush(spdlog::log_handle_t hLogger, unsigned long log_level);

	// Flushes the log file (to disk ...)
	SPDLOGLB_API bool SpdLog_LoggerFlush(spdlog::log_handle_t hLogger);

	SPDLOGLB_API bool SpdLog_GetLoggerName(spdlog::log_handle_t hLogger, char* lpBuffer, unsigned long BufSize);
	SPDLOGLB_API unsigned long SpdLog_GetLoggerLevel(spdlog::log_handle_t hLogger);
	SPDLOGLB_API unsigned long SpdLog_GetLoggerBitMask(spdlog::log_handle_t hLogger);
	SPDLOGLB_API unsigned long SpdLog_GetLoggerAutomaticFlush(spdlog::log_handle_t hLogger);

	SPDLOGLB_API bool SpdLog_SetLoggerErrorHandler(spdlog::log_handle_t hLogger,spdlog::log_err_handler ErrorHandler);

	// The log functions has the following common parameters:
	// log_level: is a value from BevlLogLevelEnum. If this value is lower than the loggers loglevel, the message is filtered out.
	// log_bit_flag: this value is compared to the mask from the logger and filtered out if the bit is not set.

	SPDLOGLB_API bool SpdLog_LogBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string);
	SPDLOGLB_API bool SpdLog_LogBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string);
	SPDLOGLB_API bool SpdLog_Log(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string);

	SPDLOGLB_API bool SpdLog_LogPrintf(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string, ...);
	SPDLOGLB_API bool SpdLog_LogPrintfBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, ...);
	SPDLOGLB_API bool SpdLog_LogPrintfBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, ...);

	SPDLOGLB_API bool SpdLog_vLogPrintf(spdlog::log_handle_t hLogger, unsigned long log_level, const spdlog::log_char_t* log_string, va_list parameter);
	SPDLOGLB_API bool SpdLog_vLogPrintfBf(spdlog::log_handle_t hLogger, unsigned long log_level, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, va_list parameter);
	SPDLOGLB_API bool SpdLog_vLogPrintfBfo(spdlog::log_handle_t hLogger, unsigned long log_bit_flag, const spdlog::log_char_t* log_string, va_list parameter);


	// Logging functions with variable parameter lists are moved to "SpdLog_VarArg.h".
	// Reason: a typesafe way to pass a variable parameter list needs includes of the format library on 
	// interfac side.


#ifdef __cplusplus
}
#endif

} // namespace spdlog

