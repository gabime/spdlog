#pragma once
#include <chrono>
#include "..\include\spdlog\details\os.h"
//#include "spdlog/tweakme.h"

#define SPDLOG_HIRES_CLOCK

namespace spdlog
{

	namespace details
	{


#ifdef SPDLOG_HIRES_CLOCK
using hires_sys_clock = std::chrono::high_resolution_clock;
#else
using hires_sys_clock = std::chrono::system_clock;
#endif

// Both types are convertible!
typedef struct hrc_filetime
{
	union
	{
		FILETIME ft;
		int64_t val;
	};
}hrc_filetime;


class hires_clock
{

private:
	// For conversion to filetime: offset in FileTime units (100 ns)
#ifdef SPDLOG_HIRES_CLOCK
	hrc_filetime _steadyclock_ft_base_offset;
#endif


public:


	hires_clock();

	inline int64_t now(void);

	inline int64_t now(unsigned long& NanoSecondsOverflow);

	inline void now(std::tm& tm);

	inline void now(std::tm& tm, unsigned long& ns_overflow);

};


class hires_clock_value : public std::tm
{
public:

	unsigned long		NanoSeconds;

	static hires_clock& get_hires_clock(void)
	{
		static hires_clock hrc;
		return hrc;
	}

	inline int64_t hires_clock_now(unsigned long& ns_overflow)
	{
		return get_hires_clock().now(ns_overflow);
	}

	inline int64_t hires_clock_now()
	{
		unsigned long uld;
		return get_hires_clock().now(uld);
	}

	hires_clock_value(int64_t TimerVal, unsigned long ns_overflow); 
	inline void Init(int64_t TimerVal, unsigned long ns_overflow);
	hires_clock_value(int64_t val);
	inline void Init(int64_t val);
	hires_clock_value();
	inline void Init(void);

};

	} // details

} // spdlog
