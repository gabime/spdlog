/*
Formatting library for C++

Copyright (c) 2012 - 2016, Victor Zverovich
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "format.h"

#include <string.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>  // for std::ptrdiff_t

#if defined(_WIN32) && defined(__MINGW32__)
# include <cstring>
#endif

#if FMT_USE_WINDOWS_H
# if defined(NOMINMAX) || defined(FMT_WIN_MINMAX)
#  include <windows.h>
# else
#  define NOMINMAX
#  include <windows.h>
#  undef NOMINMAX
# endif
#endif

using fmt::internal::Arg;

#if FMT_EXCEPTIONS
# define FMT_TRY try
# define FMT_CATCH(x) catch (x)
#else
# define FMT_TRY if (true)
# define FMT_CATCH(x) if (false)
#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)  // conditional expression is constant
# pragma warning(disable: 4702)  // unreachable code
// Disable deprecation warning for strerror. The latter is not called but
// MSVC fails to detect it.
# pragma warning(disable: 4996)
#endif

// Dummy implementations of strerror_r and strerror_s called if corresponding
// system functions are not available.
static inline fmt::internal::Null<> strerror_r(int, char *, ...)
{
	return fmt::internal::Null<>();
}
static inline fmt::internal::Null<> strerror_s(char *, std::size_t, ...)
{
	return fmt::internal::Null<>();
}

namespace fmt {

	FMT_FUNC internal::RuntimeError::~RuntimeError() FMT_DTOR_NOEXCEPT
	{}
	FMT_FUNC FormatError::~FormatError() FMT_DTOR_NOEXCEPT
	{}
	FMT_FUNC SystemError::~SystemError() FMT_DTOR_NOEXCEPT
	{}

	namespace {

#ifndef _MSC_VER
# define FMT_SNPRINTF snprintf
#else  // _MSC_VER
		inline int fmt_snprintf(char *buffer, size_t size, const char *format, ...)
		{
			va_list args;
			va_start(args, format);
			int result = vsnprintf_s(buffer, size, _TRUNCATE, format, args);
			va_end(args);
			return result;
		}
# define FMT_SNPRINTF fmt_snprintf
#endif  // _MSC_VER

#if defined(_WIN32) && defined(__MINGW32__) && !defined(__NO_ISOCEXT)
# define FMT_SWPRINTF snwprintf
#else
# define FMT_SWPRINTF swprintf
#endif // defined(_WIN32) && defined(__MINGW32__) && !defined(__NO_ISOCEXT)

		const char RESET_COLOR[] = "\x1b[0m";

		typedef void(*FormatFunc)(Writer &, int, StringRef);

		// Portable thread-safe version of strerror.
		// Sets buffer to point to a string describing the error code.
		// This can be either a pointer to a string stored in buffer,
		// or a pointer to some static immutable string.
		// Returns one of the following values:
		//   0      - success
		//   ERANGE - buffer is not large enough to store the error message
		//   other  - failure
		// Buffer should be at least of size 1.
		int safe_strerror(
			int error_code, char *&buffer, std::size_t buffer_size) FMT_NOEXCEPT
		{
			FMT_ASSERT(buffer != 0 && buffer_size != 0, "invalid buffer");

			class StrError
			{
			private:
				int error_code_;
				char *&buffer_;
				std::size_t buffer_size_;

				// A noop assignment operator to avoid bogus warnings.
				void operator=(const StrError &)
				{}

				// Handle the result of XSI-compliant version of strerror_r.
				int handle(int result)
				{
					// glibc versions before 2.13 return result in errno.
					return result == -1 ? errno : result;
				}

				// Handle the result of GNU-specific version of strerror_r.
				int handle(char *message)
				{
					// If the buffer is full then the message is probably truncated.
					if (message == buffer_ && strlen(buffer_) == buffer_size_ - 1)
						return ERANGE;
					buffer_ = message;
					return 0;
				}

				// Handle the case when strerror_r is not available.
				int handle(internal::Null<>)
				{
					return fallback(strerror_s(buffer_, buffer_size_, error_code_));
				}

				// Fallback to strerror_s when strerror_r is not available.
				int fallback(int result)
				{
					// If the buffer is full then the message is probably truncated.
					return result == 0 && strlen(buffer_) == buffer_size_ - 1 ?
						ERANGE : result;
				}

				// Fallback to strerror if strerror_r and strerror_s are not available.
				int fallback(internal::Null<>)
				{
					errno = 0;
					buffer_ = strerror(error_code_);
					return errno;
				}

			public:
				StrError(int err_code, char *&buf, std::size_t buf_size)
					: error_code_(err_code), buffer_(buf), buffer_size_(buf_size)
				{}

				int run()
				{
					// Suppress a warning about unused strerror_r.
					strerror_r(0, FMT_NULL, "");
					return handle(strerror_r(error_code_, buffer_, buffer_size_));
				}
			};
			return StrError(error_code, buffer, buffer_size).run();
		}

		void format_error_code(Writer &out, int error_code,
			StringRef message) FMT_NOEXCEPT
		{
			// Report error code making sure that the output fits into
			// INLINE_BUFFER_SIZE to avoid dynamic memory allocation and potential
			// bad_alloc.
			out.clear();
			static const char SEP[] = ": ";
			static const char ERROR_STR[] = "error ";
			// Subtract 2 to account for terminating null characters in SEP and ERROR_STR.
			std::size_t error_code_size = sizeof(SEP) + sizeof(ERROR_STR) - 2;
			typedef internal::IntTraits<int>::MainType MainType;
			MainType abs_value = static_cast<MainType>(error_code);
			if (internal::is_negative(error_code)) {
				abs_value = 0 - abs_value;
				++error_code_size;
			}
			error_code_size += internal::count_digits(abs_value);
			if (message.size() <= internal::INLINE_BUFFER_SIZE - error_code_size)
				out << message << SEP;
			out << ERROR_STR << error_code;
			assert(out.size() <= internal::INLINE_BUFFER_SIZE);
		}

		void report_error(FormatFunc func, int error_code,
			StringRef message) FMT_NOEXCEPT
		{
			MemoryWriter full_message;
			func(full_message, error_code, message);
			// Use Writer::data instead of Writer::c_str to avoid potential memory
			// allocation.
			std::fwrite(full_message.data(), full_message.size(), 1, stderr);
			std::fputc('\n', stderr);
		}
	}  // namespace

	namespace internal {

		// This method is used to preserve binary compatibility with fmt 3.0.
		// It can be removed in 4.0.
		FMT_FUNC void format_system_error(
			Writer &out, int error_code, StringRef message) FMT_NOEXCEPT
		{
			fmt::format_system_error(out, error_code, message);
		}
	}  // namespace internal

	FMT_FUNC void SystemError::init(
		int err_code, CStringRef format_str, ArgList args)
	{
		error_code_ = err_code;
		MemoryWriter w;
		format_system_error(w, err_code, format(format_str, args));
		std::runtime_error &base = *this;
		base = std::runtime_error(w.str());
	}

	template <typename T>
	int internal::CharTraits<char>::format_float(
		char *buffer, std::size_t size, const char *format,
		unsigned width, int precision, T value)
	{
		if (width == 0) {
			return precision < 0 ?
				FMT_SNPRINTF(buffer, size, format, value) :
				FMT_SNPRINTF(buffer, size, format, precision, value);
		}
		return precision < 0 ?
			FMT_SNPRINTF(buffer, size, format, width, value) :
			FMT_SNPRINTF(buffer, size, format, width, precision, value);
	}

	template <typename T>
	int internal::CharTraits<wchar_t>::format_float(
		wchar_t *buffer, std::size_t size, const wchar_t *format,
		unsigned width, int precision, T value)
	{
		if (width == 0) {
			return precision < 0 ?
				FMT_SWPRINTF(buffer, size, format, value) :
				FMT_SWPRINTF(buffer, size, format, precision, value);
		}
		return precision < 0 ?
			FMT_SWPRINTF(buffer, size, format, width, value) :
			FMT_SWPRINTF(buffer, size, format, width, precision, value);
	}

	template <typename T>
	const char internal::BasicData<T>::DIGITS[] =
		"0001020304050607080910111213141516171819"
		"2021222324252627282930313233343536373839"
		"4041424344454647484950515253545556575859"
		"6061626364656667686970717273747576777879"
		"8081828384858687888990919293949596979899";

#define FMT_POWERS_OF_10(factor) \
  factor * 10, \
  factor * 100, \
  factor * 1000, \
  factor * 10000, \
  factor * 100000, \
  factor * 1000000, \
  factor * 10000000, \
  factor * 100000000, \
  factor * 1000000000

	template <typename T>
	const uint32_t internal::BasicData<T>::POWERS_OF_10_32[] = {
		0, FMT_POWERS_OF_10(1)
	};

	template <typename T>
	const uint64_t internal::BasicData<T>::POWERS_OF_10_64[] = {
		0,
		FMT_POWERS_OF_10(1),
		FMT_POWERS_OF_10(ULongLong(1000000000)),
		// Multiply several constants instead of using a single long long constant
		// to avoid warnings about C++98 not supporting long long.
		ULongLong(1000000000) * ULongLong(1000000000) * 10
	};

	FMT_FUNC void internal::report_unknown_type(char code, const char *type)
	{
		(void)type;
		if (std::isprint(static_cast<unsigned char>(code))) {
			FMT_THROW(FormatError(
				format("unknown format code '{}' for {}", code, type)));
		}
		FMT_THROW(FormatError(
			format("unknown format code '\\x{:02x}' for {}",
				static_cast<unsigned>(code), type)));
	}

#if FMT_USE_WINDOWS_H

	FMT_FUNC internal::UTF8ToUTF16::UTF8ToUTF16(StringRef s)
	{
		static const char ERROR_MSG[] = "cannot convert string from UTF-8 to UTF-16";
		if (s.size() > INT_MAX)
			FMT_THROW(WindowsError(ERROR_INVALID_PARAMETER, ERROR_MSG));
		int s_size = static_cast<int>(s.size());
		int length = MultiByteToWideChar(
			CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), s_size, FMT_NULL, 0);
		if (length == 0)
			FMT_THROW(WindowsError(GetLastError(), ERROR_MSG));
		buffer_.resize(length + 1);
		length = MultiByteToWideChar(
			CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), s_size, &buffer_[0], length);
		if (length == 0)
			FMT_THROW(WindowsError(GetLastError(), ERROR_MSG));
		buffer_[length] = 0;
	}

	FMT_FUNC internal::UTF16ToUTF8::UTF16ToUTF8(WStringRef s)
	{
		if (int error_code = convert(s)) {
			FMT_THROW(WindowsError(error_code,
				"cannot convert string from UTF-16 to UTF-8"));
		}
	}

	FMT_FUNC int internal::UTF16ToUTF8::convert(WStringRef s)
	{
		if (s.size() > INT_MAX)
			return ERROR_INVALID_PARAMETER;
		int s_size = static_cast<int>(s.size());
		int length = WideCharToMultiByte(
			CP_UTF8, 0, s.data(), s_size, FMT_NULL, 0, FMT_NULL, FMT_NULL);
		if (length == 0)
			return GetLastError();
		buffer_.resize(length + 1);
		length = WideCharToMultiByte(
			CP_UTF8, 0, s.data(), s_size, &buffer_[0], length, FMT_NULL, FMT_NULL);
		if (length == 0)
			return GetLastError();
		buffer_[length] = 0;
		return 0;
	}

	FMT_FUNC void WindowsError::init(
		int err_code, CStringRef format_str, ArgList args)
	{
		error_code_ = err_code;
		MemoryWriter w;
		internal::format_windows_error(w, err_code, format(format_str, args));
		std::runtime_error &base = *this;
		base = std::runtime_error(w.str());
	}

	FMT_FUNC void internal::format_windows_error(
		Writer &out, int error_code, StringRef message) FMT_NOEXCEPT
	{
		FMT_TRY{
			MemoryBuffer<wchar_t, INLINE_BUFFER_SIZE> buffer;
		buffer.resize(INLINE_BUFFER_SIZE);
		for (;;) {
			wchar_t *system_message = &buffer[0];
			int result = FormatMessageW(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				FMT_NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				system_message, static_cast<uint32_t>(buffer.size()), FMT_NULL);
			if (result != 0) {
				UTF16ToUTF8 utf8_message;
				if (utf8_message.convert(system_message) == ERROR_SUCCESS) {
					out << message << ": " << utf8_message;
					return;
				}
				break;
			}
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				break;  // Can't get error message, report error code instead.
			buffer.resize(buffer.size() * 2);
		}
		} FMT_CATCH(...)
		{}
		fmt::format_error_code(out, error_code, message);  // 'fmt::' is for bcc32.
	}

#endif  // FMT_USE_WINDOWS_H

	FMT_FUNC void format_system_error(
		Writer &out, int error_code, StringRef message) FMT_NOEXCEPT
	{
		FMT_TRY{
			internal::MemoryBuffer<char, internal::INLINE_BUFFER_SIZE> buffer;
		buffer.resize(internal::INLINE_BUFFER_SIZE);
		for (;;) {
			char *system_message = &buffer[0];
			int result = safe_strerror(error_code, system_message, buffer.size());
			if (result == 0) {
				out << message << ": " << system_message;
				return;
			}
			if (result != ERANGE)
				break;  // Can't get error message, report error code instead.
			buffer.resize(buffer.size() * 2);
		}
		} FMT_CATCH(...)
		{}
		fmt::format_error_code(out, error_code, message);  // 'fmt::' is for bcc32.
	}

	template <typename Char>
	void internal::ArgMap<Char>::init(const ArgList &args)
	{
		if (!map_.empty())
			return;
		typedef internal::NamedArg<Char> NamedArg;
		const NamedArg *named_arg = FMT_NULL;
		bool use_values =
			args.type(ArgList::MAX_PACKED_ARGS - 1) == internal::Arg::NONE;
		if (use_values) {
			for (unsigned i = 0;/*nothing*/; ++i) {
				internal::Arg::Type arg_type = args.type(i);
				switch (arg_type) {
				case internal::Arg::NONE:
					return;
				case internal::Arg::NAMED_ARG:
					named_arg = static_cast<const NamedArg*>(args.values_[i].pointer);
					map_.push_back(Pair(named_arg->name, *named_arg));
					break;
				default:
					/*nothing*/;
				}
			}
			return;
		}
		for (unsigned i = 0; i != ArgList::MAX_PACKED_ARGS; ++i) {
			internal::Arg::Type arg_type = args.type(i);
			if (arg_type == internal::Arg::NAMED_ARG) {
				named_arg = static_cast<const NamedArg*>(args.args_[i].pointer);
				map_.push_back(Pair(named_arg->name, *named_arg));
			}
		}
		for (unsigned i = ArgList::MAX_PACKED_ARGS;/*nothing*/; ++i) {
			switch (args.args_[i].type) {
			case internal::Arg::NONE:
				return;
			case internal::Arg::NAMED_ARG:
				named_arg = static_cast<const NamedArg*>(args.args_[i].pointer);
				map_.push_back(Pair(named_arg->name, *named_arg));
				break;
			default:
				/*nothing*/;
			}
		}
	}

	template <typename Char>
	void internal::FixedBuffer<Char>::grow(std::size_t)
	{
		FMT_THROW(std::runtime_error("buffer overflow"));
	}

	FMT_FUNC Arg internal::FormatterBase::do_get_arg(
		unsigned arg_index, const char *&error)
	{
		Arg arg = args_[arg_index];
		switch (arg.type) {
		case Arg::NONE:
			error = "argument index out of range";
			break;
		case Arg::NAMED_ARG:
			arg = *static_cast<const internal::Arg*>(arg.pointer);
			break;
		default:
			/*nothing*/;
		}
		return arg;
	}

	FMT_FUNC void report_system_error(
		int error_code, fmt::StringRef message) FMT_NOEXCEPT
	{
		// 'fmt::' is for bcc32.
		report_error(format_system_error, error_code, message);
	}

#if FMT_USE_WINDOWS_H
	FMT_FUNC void report_windows_error(
		int error_code, fmt::StringRef message) FMT_NOEXCEPT
	{
		// 'fmt::' is for bcc32.
		report_error(internal::format_windows_error, error_code, message);
	}
#endif

	FMT_FUNC void print(std::FILE *f, CStringRef format_str, ArgList args)
	{
		MemoryWriter w;
		w.write(format_str, args);
		std::fwrite(w.data(), 1, w.size(), f);
	}

	FMT_FUNC void print(CStringRef format_str, ArgList args)
	{
		print(stdout, format_str, args);
	}

	FMT_FUNC void print_colored(Color c, CStringRef format, ArgList args)
	{
		char escape[] = "\x1b[30m";
		escape[3] = static_cast<char>('0' + c);
		std::fputs(escape, stdout);
		print(format, args);
		std::fputs(RESET_COLOR, stdout);
	}

#ifndef FMT_HEADER_ONLY

	template struct internal::BasicData<void>;

	// Explicit instantiations for char.

	template void internal::FixedBuffer<char>::grow(std::size_t);

	template void internal::ArgMap<char>::init(const ArgList &args);

	template int internal::CharTraits<char>::format_float(
		char *buffer, std::size_t size, const char *format,
		unsigned width, int precision, double value);

	template int internal::CharTraits<char>::format_float(
		char *buffer, std::size_t size, const char *format,
		unsigned width, int precision, long double value);

	// Explicit instantiations for wchar_t.

	template void internal::FixedBuffer<wchar_t>::grow(std::size_t);

	template void internal::ArgMap<wchar_t>::init(const ArgList &args);

	template int internal::CharTraits<wchar_t>::format_float(
		wchar_t *buffer, std::size_t size, const wchar_t *format,
		unsigned width, int precision, double value);

	template int internal::CharTraits<wchar_t>::format_float(
		wchar_t *buffer, std::size_t size, const wchar_t *format,
		unsigned width, int precision, long double value);

#endif  // FMT_HEADER_ONLY

}  // namespace fmt

#ifdef _MSC_VER
# pragma warning(pop)
#endif
