#pragma once

#include "cstyle\vararg_list.h"
#include "spdloglb.h"

namespace spdlog
{


#ifdef __cplusplus
	extern "C" {
#endif
		// Logs a message with parameters using the formating library done by Victor Zverovich ({} style)
	// (http://zverovich.net/2012/12/12/a-better-string-formatting-library-for-cplusplus.html)
	// (https://github.com/fmtlib/fmt)
	// 
	// This functions need to receive the variable parameter list as a vararg_list object. This object is designed in a way 
	// that it is possible to construct it by providing a chain of up to 21 values in a way that is similar to a vararg (...) list.

		SPDLOGLB_API bool SpdLog_LogFormat(spdlog::log_handle_t hLogger, unsigned long LogLevel, const spdlog::log_char_t* LogString, vararg_list& args);
		SPDLOGLB_API bool SpdLog_LogFormatBf(spdlog::log_handle_t hLogger, unsigned long LogLevel, unsigned long LogBitFlag, const spdlog::log_char_t*  LogString, vararg_list& args);
		SPDLOGLB_API bool SpdLog_LogFormatBfo(spdlog::log_handle_t hLogger, unsigned long LogBitFlag, const spdlog::log_char_t*  LogString, vararg_list& args);

		// Taken out for the moment, does not work with ghe current vuildin version of the fmt library.
		// Seems to work with the last version of fmt.
		//// The same way, just with the printf style format syntax of the format library.
		//// Attention: the format specifiers are not neccessarily the same that you know from "printf", look into the doku 
		//// of the format library. For example, "%S" for a string of the opposite character format (char/wchar) is not supported.
		//SPDLOGLB_API bool SpdLog_LogFormatPrintf(spdlog::log_handle_t hLogger, unsigned long LogLevel, const spdlog::log_char_t*  LogString, vararg_list& args);
		//SPDLOGLB_API bool SpdLog_LogFormatPrintfBf(spdlog::log_handle_t hLogger, unsigned long LogLevel, unsigned long LogBitFlag, const spdlog::log_char_t* LogString, vararg_list& args);
		//SPDLOGLB_API bool SpdLog_LogFormatPrintfBfo(spdlog::log_handle_t hLogger, unsigned long LogBitFlag, const spdlog::log_char_t*  LogString, vararg_list& args);
#ifdef __cplusplus
	}
#endif

const unsigned long VAIS_DEFAULT_ARGUMENT_COUNT = 10;


// This objects can be send into a vararg_output_stream object using the operator "<<".
// They have to be the last element of the output chain behind the parameters. 
// This looks better, but has got the cost of creating a temporary object with a copy of the required parameters.
class VOS_WRITE
{
public:
	unsigned long				_LogLevel; 
	const spdlog::log_char_t*	_FormatString;

	VOS_WRITE(unsigned long LogLevel, const spdlog::log_char_t* FormatString) {_LogLevel = LogLevel; _FormatString = FormatString;}

};

class VOS_WRITE_BF
{
public:
	unsigned long				_LogLevel;
	const spdlog::log_char_t*	_FormatString;
	unsigned long				_LogBitFlag;
	VOS_WRITE_BF(unsigned long LogLevel, unsigned long LogBitFlag, const spdlog::log_char_t*  FormatString)
	{_LogLevel = LogLevel;_FormatString = FormatString;_LogBitFlag = LogBitFlag;}

};

class VOS_WRITE_BFO
{
public:
	const spdlog::log_char_t*	_FormatString;
	unsigned long				_LogBitFlag;
	VOS_WRITE_BFO(unsigned long LogBitFlag, const spdlog::log_char_t*  FormatString)
	{
		_FormatString = FormatString; _LogBitFlag = LogBitFlag;
	}

};



/******************************************************************************
Represents a logging format stream with a variable parameter list. You can stream
the parameters in with the stream operator "<<", then you call one of the write
functions with the format string and other required parameters. After the write
operation the parameter list is empty again for the next operation.

It is recommended to create just one object for each logger and keep it until 
you shutdown logging. The parameter list is optimized to minimize memory operations
(grow). It would be inappropriate to create and destroy this list with each 
logging operation.

Usage: Create the output stream object with a valid logger handle.

Essential: you need to store the variable parameters first, then call the 
logging function with the formatstring. First the parameters, then the operation!

*******************************************************************************/
	class vararg_output_stream
	{
	private:
		vararg_list				_arguments;
		spdlog::log_handle_t	_h_logger;


	public:
		explicit vararg_output_stream(spdlog::log_handle_t h_logger) :
			_arguments(VAIS_DEFAULT_ARGUMENT_COUNT)
		{
			_h_logger = h_logger;
		}
		explicit vararg_output_stream(void) :
			_arguments(VAIS_DEFAULT_ARGUMENT_COUNT)
		{
			_h_logger = nullptr;
		}

		inline spdlog::log_handle_t  get_logger_handle(void) { return _h_logger; }
		
		inline void set_logger_handle(spdlog::log_handle_t h_logger) { _h_logger = h_logger; }

		// Attention: stream in the required arguments FIRST!
		inline bool Write(unsigned long LogLevel, const spdlog::log_char_t* LogString)
		{
			bool rv = SpdLog_LogFormat(_h_logger, LogLevel, LogString, _arguments);
			_arguments.clear();
			return rv;
		}
		// Attention: stream in the required arguments FIRST!
		inline bool WriteBf(unsigned long LogLevel, unsigned long LogBitFlag, const spdlog::log_char_t*  LogString)
		{
			bool rv = SpdLog_LogFormatBf(_h_logger, LogLevel, LogBitFlag, LogString, _arguments);
			_arguments.clear();
			return rv;
		}
		// Attention: stream in the required arguments FIRST!
		inline bool WriteBfo(unsigned long LogBitFlag, const spdlog::log_char_t*  LogString)
		{
			bool rv = SpdLog_LogFormatBfo(_h_logger,  LogBitFlag, LogString, _arguments);
			_arguments.clear();
			return rv;
		}

		// the stream operators...

		vararg_output_stream& operator<<(VOS_WRITE& data)
		{
			Write(data._LogLevel, data._FormatString);
			return *this;
		}

		vararg_output_stream& operator<<(VOS_WRITE_BF& data)
		{
			WriteBf(data._LogLevel,data._LogBitFlag, data._FormatString);
			return *this;
		}

		vararg_output_stream& operator<<(VOS_WRITE_BFO& data)
		{
			WriteBfo(data._LogBitFlag, data._FormatString);
			return *this;
		}

		vararg_output_stream& operator<<(fmt::internal::Arg& data)
		{
			_arguments.add_arg(data);
			return *this;
		}

		vararg_output_stream& operator<<(int data)
		{
			_arguments.add_int(data);
			return *this;
		}

		vararg_output_stream& operator<<(unsigned int data)
		{
			_arguments.add_uint(data);
			return *this;
		}

		vararg_output_stream& operator<<(long data)
		{
			_arguments.add_int(data);
			return *this;
		}

		vararg_output_stream& operator<<(unsigned long data)
		{
			_arguments.add_uint(data);
			return *this;
		}


		vararg_output_stream& operator<<(long long data)
		{
			_arguments.add_longlong(data);
			return *this;
		}

		vararg_output_stream& operator<<(unsigned long long data)
		{
			_arguments.add_ulonglong(data);
			return *this;
		}

		vararg_output_stream& operator<<(double data)
		{
			_arguments.add_double(data);
			return *this;
		}

		vararg_output_stream& operator<<(long double data)
		{
			_arguments.add_long_double(data);
			return *this;
		}

		vararg_output_stream& operator<<(const void* data)
		{
			_arguments.add_pointer(data);
			return *this;
		}

		vararg_output_stream& operator<<(vararg_sstring& data)
		{
			_arguments.add_sstring(data);
			return *this;
		}



		vararg_output_stream& operator<<(const char* data)
		{
			_arguments.add_sstring(data);
			return *this;
		}

		vararg_output_stream& operator<<(vararg_ustring& data)
		{
			_arguments.add_ustring(data);
			return *this;
		}

		vararg_output_stream& operator<<(const unsigned char* data)
		{
			_arguments.add_ustring(data);
			return *this;
		}

		vararg_output_stream& operator<<(const wchar_t* data)
		{
			_arguments.add_wstring(data);
			return *this;
		}

		vararg_output_stream& operator<<(vararg_wstring& data)
		{
			_arguments.add_wstring(data);
			return *this;
		}

		vararg_output_stream& operator<<(char data)
		{
			_arguments.add_char(data);
			return *this;
		}

		vararg_output_stream& operator<<(bool data)
		{
			_arguments.add_bool(data);
			return *this;
		}


	};

} // namespace spdlog