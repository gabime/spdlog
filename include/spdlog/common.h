//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#define SPDLOG_VERSION "0.16.4-rc"

#include "tweakme.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>

#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
#include <codecvt>
#include <locale>
#endif

#include "details/null_mutex.h"

// visual studio upto 2013 does not support noexcept nor constexpr
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define SPDLOG_NOEXCEPT throw()
#define SPDLOG_CONSTEXPR
#else
#define SPDLOG_NOEXCEPT noexcept
#define SPDLOG_CONSTEXPR constexpr
#endif

// final keyword support. On by default. See tweakme.h
#if defined(SPDLOG_NO_FINAL)
#define SPDLOG_FINAL
#else
#define SPDLOG_FINAL final
#endif

#if defined(__GNUC__) || defined(__clang__)
#define SPDLOG_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define SPDLOG_DEPRECATED __declspec(deprecated)
#else
#define SPDLOG_DEPRECATED
#endif

#include "fmt/fmt.h"

namespace spdlog {

	struct base_allocator {
		using alloc = void*(*)(size_t);
		using free = void(*)(void*);

		alloc allocate{ std::malloc };
		free deallocate{ std::free };
	};
	inline base_allocator& allocator()
	{
		static base_allocator allocator;
		return allocator;
	}

	template<class T>
	struct _allocator
	{
		using value_type = T;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = value_type&;
		using const_reference = const value_type&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		_allocator() = default;
		_allocator(const _allocator&) = default;
		_allocator(_allocator&&) = default;
		template <class U> constexpr _allocator(const _allocator<U>&) noexcept {}

		T* allocate(size_t n)
		{
			if (n > size_t(-1) / sizeof(T)) return nullptr;
			if (auto p = static_cast<T*>(allocator().allocate(n * sizeof(T)))) return p;
			return nullptr;
		}
		void deallocate(T* p, size_t) noexcept { allocator().deallocate(p); }

		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }
		size_type max_size() const { return static_cast<size_type>(-1) / sizeof(value_type); }
		template< class U, class... Args >
		void construct(U* p, Args&&... args) { new((void *)p) U(std::forward<Args>(args)...); }
		void destroy(pointer p) { p->~value_type(); (void)p; }
	};

	template <class T, class U>
	bool operator==(const _allocator<T>&, const _allocator<U>&) { return true; }
	template <class T, class U>
	bool operator!=(const _allocator<T>&, const _allocator<U>&) { return false; }

	using string = std::basic_string<char, std::char_traits<char>, _allocator<char>>;
	using wstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, _allocator<wchar_t>>;
	template<typename KEY, typename VALUE, class HASHER = std::hash<KEY>, class COMP = std::equal_to<KEY>>
	using unordered_map = std::unordered_map<KEY, VALUE, HASHER, COMP, _allocator<std::pair<KEY, VALUE>>>;
	template<typename T>
	using vector = std::vector<T, _allocator<T>>;

	template<class T>
	struct _deleter {
		void operator()(T* p) const {
			p->~T();
			allocator().deallocate(p);
		}

		constexpr _deleter() = default;
		constexpr _deleter(const _deleter&) = default;
		constexpr _deleter(_deleter&&) = default;
		_deleter& operator=(const _deleter&) = default;
		_deleter& operator=(_deleter&&) = default;
		~_deleter() = default;
		template<class T2,
			class = typename std::enable_if<std::is_convertible<T2*, T*>::value,
			void>::type>
			_deleter(const _deleter<T2>&)
		{	// construct from another default_delete
		}
	};
	template<typename TYPE>
	using unique_ptr = std::unique_ptr<TYPE, _deleter<TYPE>>;
	template<class T, class... ARGS>
	unique_ptr<T> make_unique(ARGS&&... args)
	{
		auto p = allocator().allocate(sizeof(T));
		T* ptr = new (p) T(std::forward<ARGS>(args)...);
		return unique_ptr<T>(ptr);
	}
	template<class T>
	using shared_ptr = std::shared_ptr<T>;
	template<class T, class... Args>
	shared_ptr<T> make_shared(Args&&... args)
	{
		return std::allocate_shared<T>(_allocator<T>(), std::forward<Args>(args)...);
	}

class formatter;

namespace sinks {
class sink;
}

using log_clock = std::chrono::system_clock;
using sink_ptr = shared_ptr<sinks::sink>;
using sinks_init_list = std::initializer_list<sink_ptr>;
using formatter_ptr = shared_ptr<spdlog::formatter>;
#if defined(SPDLOG_NO_ATOMIC_LEVELS)
using level_t = details::null_atomic_int;
#else
using level_t = std::atomic<int>;
#endif

using log_err_handler = std::function<void(const string &err_msg)>;

// Log level enum
namespace level {
enum level_enum
{
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    err = 4,
    critical = 5,
    off = 6
};

#if !defined(SPDLOG_LEVEL_NAMES)
#define SPDLOG_LEVEL_NAMES                                                                                                                 \
    {                                                                                                                                      \
        "trace", "debug", "info", "warning", "error", "critical", "off"                                                                    \
    }
#endif
static const char *level_names[] SPDLOG_LEVEL_NAMES;

static const char *short_level_names[]{"T", "D", "I", "W", "E", "C", "O"};

inline const char *to_str(spdlog::level::level_enum l)
{
    return level_names[l];
}

inline const char *to_short_str(spdlog::level::level_enum l)
{
    return short_level_names[l];
}
inline spdlog::level::level_enum from_str(const string &name)
{
    static unordered_map<string, level_enum> name_to_level = // map string->level
        {{level_names[0], level::trace},                     // trace
            {level_names[1], level::debug},                  // debug
            {level_names[2], level::info},                   // info
            {level_names[3], level::warn},                   // warn
            {level_names[4], level::err},                    // err
            {level_names[5], level::critical},               // critical
            {level_names[6], level::off}};                   // off

    auto lvl_it = name_to_level.find(name);
    return lvl_it != name_to_level.end() ? lvl_it->second : level::off;
}

using level_hasher = std::hash<int>;
} // namespace level

//
// Async overflow policy - block by default.
//
enum class async_overflow_policy
{
    block_retry,    // Block / yield / sleep until message can be enqueued
    discard_log_msg // Discard the message it enqueue fails
};

//
// Pattern time - specific time getting to use for pattern_formatter.
// local time by default
//
enum class pattern_time_type
{
    local, // log localtime
    utc    // log utc
};

//
// Log exception
//
namespace details {
namespace os {
string errno_str(int err_num);
}
} // namespace details
class spdlog_ex : public std::exception
{
public:
    explicit spdlog_ex(string msg)
        : _msg(std::move(msg))
    {
    }

    spdlog_ex(const string &msg, int last_errno)
    {
        _msg = msg + ": " + details::os::errno_str(last_errno);
    }

    const char *what() const SPDLOG_NOEXCEPT override
    {
        return _msg.c_str();
    }

private:
    string _msg;
};

//
// wchar support for windows file names (SPDLOG_WCHAR_FILENAMES must be defined)
//
#if defined(_WIN32) && defined(SPDLOG_WCHAR_FILENAMES)
using filename_t = wstring;
#else
using filename_t = string;
#endif

} // namespace spdlog
