#pragma once
#include <chrono>
#include "..\include\spdlog\details\os.h"
//#include "spdlog/tweakme.h"
#include "hires_clock.h"
#include "spdlog/common.h"


namespace spdlog
{

	namespace details
	{

		hires_clock::hires_clock()
		{

#ifdef SPDLOG_HIRES_CLOCK

			hrc_filetime ftSys, ftLoc;
			GetSystemTimeAsFileTime(&ftSys.ft);
			FileTimeToLocalFileTime(&ftSys.ft, &ftLoc.ft);

			std::chrono::time_point<hires_sys_clock, std::chrono::nanoseconds> tpn = hires_sys_clock::now();
			int64_t SteadyValNs = tpn.time_since_epoch().count() / 100;
			_steadyclock_ft_base_offset.val = ftLoc.val - SteadyValNs;

#endif // SPDLOG_HIRES_CLOCK
		}



		inline int64_t hires_clock::now(void)
		{
#ifdef SPDLOG_HIRES_CLOCK
			int64_t res = _steadyclock_ft_base_offset.val + (hires_sys_clock::now().time_since_epoch().count() / 100);
			return res;
#else // SPDLOG_HIRES_CLOCK
			return hires_sys_clock::now().time_since_epoch().count();
#endif // SPDLOG_HIRES_CLOCK
		}

		inline int64_t hires_clock::now(unsigned long& NanoSecondsOverflow)
		{
#ifdef SPDLOG_HIRES_CLOCK
			std::chrono::time_point<hires_sys_clock, std::chrono::nanoseconds> tpn = hires_sys_clock::now();
			int64_t res = tpn.time_since_epoch().count() / 100;
			NanoSecondsOverflow = tpn.time_since_epoch().count() % 100;
			res += _steadyclock_ft_base_offset.val;
			return res;
#else // SPDLOG_HIRES_CLOCK
			return hires_sys_clock::now().time_since_epoch().count();
#endif // SPDLOG_HIRES_CLOCK
		}

		inline void hires_clock::now(std::tm& tm)
		{
#ifdef SPDLOG_HIRES_CLOCK
			unsigned long no;
			now(tm, no);
#else // SPDLOG_HIRES_CLOCK
			tm = localtime(hires_sys_clock::now().time_since_epoch().count());
#endif // SPDLOG_HIRES_CLOCK
		}

		inline void hires_clock::now(std::tm& tm, unsigned long& ns_overflow)
		{
#ifdef SPDLOG_HIRES_CLOCK
			hrc_filetime ft;
			ft.val = now(ns_overflow);
			SYSTEMTIME st;
			FileTimeToSystemTime(&ft.ft, &st);
			tm.tm_hour = st.wHour;
			tm.tm_isdst = false;			// Question: how to determine?
			tm.tm_mday = st.wDay;
			tm.tm_min = st.wMinute;
			tm.tm_mon = st.wMonth;
			tm.tm_sec = st.wSecond;
			tm.tm_wday = st.wDayOfWeek;
			tm.tm_yday = 0;				// Not used as long to formate dates in SpeedLog
			tm.tm_year = st.wYear;
#else // SPDLOG_HIRES_CLOCK
			hires_sys_clock::time_point duration = hires_sys_clock::now();
			tm = localtime(hires_sys_clock::to_time_t(duration));
			ns_overflow = std::chrono::duration_cast<std::chrono::nanoseconds>(duration.time_since_epoch()).count() % 1000000000;
#endif // SPDLOG_HIRES_CLOCK
		}







		hires_clock_value::hires_clock_value(int64_t TimerVal, unsigned long ns_overflow) { Init(TimerVal, ns_overflow); }

		inline void hires_clock_value::Init(int64_t TimerVal, unsigned long ns_overflow)
		{
			Init(TimerVal);
			NanoSeconds += ns_overflow;
		}


		hires_clock_value::hires_clock_value(int64_t val) { Init(val); }

		inline void hires_clock_value::Init(int64_t val)
		{

			hrc_filetime ft;
			ft.val = val;

			// Gain all the infos about dates and all using system functionality
			SYSTEMTIME st;
			FileTimeToSystemTime(&ft.ft, &st);
			this->tm_hour = st.wHour;
			this->tm_isdst = false;			// Question: how to determine?
			this->tm_mday = st.wDay;
			this->tm_min = st.wMinute;
			this->tm_mon = st.wMonth;
			this->tm_sec = st.wSecond;
			this->tm_wday = st.wDayOfWeek;
			this->tm_yday = 0;				// Not used as long to formate dates in SpeedLog
			this->tm_year = st.wYear;

			// Now get the nanoseconds. 
			// 7 decimal points for ns (100 ns steps caused by base on FILETIME)
			this->NanoSeconds = (ft.val % 10000000) * 100;
		}

		hires_clock_value::hires_clock_value() { Init(); }

		inline void hires_clock_value::Init(void)
		{

			unsigned long ns_overflow = 0;
#ifdef SPDLOG_HIRES_CLOCK
			int64_t cln = hires_clock_now(ns_overflow);
			Init(cln, ns_overflow);
#else // SPDLOG_HIRES_CLOCK
			get_hires_clock().now(*this, this->NanoSeconds);
#endif // SPDLOG_HIRES_CLOCK

		}


} // details

} // spdlog
