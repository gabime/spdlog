#pragma once
#include <unordered_map>
#include <mutex>
#include <algorithm> // std::transform
#include <cctype> // std::tolower
#include <set>
#include "spdlog_handle.h"
#include "..\common.h"
#include "..\sinks\ostream_sink.h"
#include "..\sinks\stdout_sinks.h"
#include "..\sinks\ansicolor_sink.h"
#include "..\sinks\msvc_sink.h"
#include "..\sinks\file_sinks.h"

namespace spdlog
{

	class sink_handle
	{
	private:
		spdlog::sink_ptr	_lpSink;

	public:
		sink_handle(spdlog::sink_ptr& sp){_lpSink = sp;}
		~sink_handle(){_lpSink.reset();}

		spdlog::log_handle_t GetHandle(void) { return this; }
		sinks::sink* GetSinkPtr(void) { return _lpSink.get(); }
		spdlog::sink_ptr& GetSinkSharedPointer(void) { return _lpSink; }
	};

//	using sink_handle_ptr = std::shared_ptr<sink_handle>;

	class sink_handle_factory
	{
	private:

		using sink_handle_list_t = std::unordered_map<spdlog::log_handle_t, sink_handle*>;



		// Static members: handle list and guard mutex etc.
		// Trick: use statics wrapped with functions
		static sink_handle_list_t& _sink_handle_list() { static sink_handle_list_t MyVal; return MyVal; }
		static std::mutex& _lock_mutex() { static std::mutex MyVal; return MyVal; }

	public:

		static void drop_all(void)
		{
			sink_handle_list_t::iterator it;
			for (it = _sink_handle_list().begin(); it != _sink_handle_list().end(); it++)
			{
				if (it->second) delete it->second;
				it->second = NULL;
			}
			_sink_handle_list().clear();
		}

		static inline sink_handle* get_sink(spdlog::log_handle_t hSink)
		{
			std::lock_guard<std::mutex> lock(_lock_mutex());
			sink_handle_list_t::iterator it = _sink_handle_list().find(hSink);
			if (it != _sink_handle_list().end())
			{
				return it->second;
			}
			return nullptr;
		}


		static sinks::sink* get_sink_ptr(spdlog::log_handle_t hSink)
		{
			std::lock_guard<std::mutex> lock(_lock_mutex());
			sink_handle_list_t::iterator it = _sink_handle_list().find(hSink);
			if (it != _sink_handle_list().end())
			{
				if (it->second)
					return it->second->GetSinkPtr();
			}
			return nullptr;
		}



		static bool free_sink(spdlog::log_handle_t hSink)
		{
			std::lock_guard<std::mutex> lock(_lock_mutex());

			sink_handle_list_t::iterator it = _sink_handle_list().find(hSink);
			if (it != _sink_handle_list().end())
			{
				if (it->second) delete it->second;

				_sink_handle_list().erase(it);
				return true;
			}
			return false;
		}

		static spdlog::log_handle_t create_ostream_sink(spdlog::std_ostream_t& os, bool MultiThreaded, bool force_flush)
		{

			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::ostream_sink_mt>(os, force_flush);
			else
				ps = std::make_shared<spdlog::sinks::ostream_sink_st>(os, force_flush);

			std::lock_guard<std::mutex> lock(_lock_mutex());

			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();
		}


		static spdlog::log_handle_t create_stderr_sink(bool MultiThreaded, bool UseColor)
		{

			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = spdlog::sinks::stderr_sink_mt::instance();
			else
				ps = spdlog::sinks::stderr_sink_st::instance();

			if (UseColor) //use color wrapper sink
				ps = std::make_shared<spdlog::sinks::ansicolor_sink>(ps);

			std::lock_guard<std::mutex> lock(_lock_mutex());

			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();

		}

		static spdlog::log_handle_t create_stdout_sink(bool MultiThreaded, bool UseColor)
		{
			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = spdlog::sinks::stdout_sink_mt::instance();
			else
				ps = spdlog::sinks::stdout_sink_st::instance();

			if (UseColor) //use color wrapper sink
				ps = std::make_shared<spdlog::sinks::ansicolor_sink>(ps);

			std::lock_guard<std::mutex> lock(_lock_mutex());

			// Sinks like Ostream, Stdout, Stderr, Msvc has a 1:1 pairing with a single multi_ref_sink object.
			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();

		}

		static spdlog::log_handle_t  create_msvc_sink(bool MultiThreaded)
		{

			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::msvc_sink_mt>();
			else
				ps = std::make_shared<spdlog::sinks::msvc_sink_st>();

			std::lock_guard<std::mutex> lock(_lock_mutex());

			// Sinks like Ostream, Stdout, Stderr, Msvc has a 1:1 pairing with a single multi_ref_sink object.
			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();

		}


		static spdlog::log_handle_t create_daily_file_sink(spdlog::filename_t FileName, spdlog::filename_t FileExt, int rotation_hour, int rotation_minute,  bool MultiThreaded)
		{
			std::lock_guard<std::mutex> lock(_lock_mutex());


			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::daily_file_sink_mt>(FileName, FileExt, rotation_hour, rotation_minute);
			else
				ps = std::make_shared<spdlog::sinks::daily_file_sink_st>(FileName, FileExt, rotation_hour, rotation_minute);

			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();

		}


		static spdlog::log_handle_t create_rotating_file_sink(spdlog::filename_t fnc, spdlog::filename_t fnx, bool MultiThreaded, size_t max_file_size, size_t max_files)
		{
			std::lock_guard<std::mutex> lock(_lock_mutex());

			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fnc, fnx, max_file_size, max_files);
			else
				ps = std::make_shared<spdlog::sinks::rotating_file_sink_st>(fnc, fnx, max_file_size, max_files);


			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();

		}



		static spdlog::log_handle_t create_simple_file_sink(spdlog::filename_t fnc, bool force_flush, bool MultiThreaded)
		{

			std::lock_guard<std::mutex> lock(_lock_mutex());


			spdlog::sink_ptr ps;
			if (MultiThreaded)
				ps = std::make_shared<spdlog::sinks::simple_file_sink_mt>(fnc, force_flush);
			else
				ps = std::make_shared<spdlog::sinks::simple_file_sink_st>(fnc, force_flush);

			sink_handle* lpSh = new sink_handle(ps);
			_sink_handle_list().insert(sink_handle_list_t::value_type(lpSh->GetHandle(), lpSh));
			return lpSh->GetHandle();

		}



	};

} // spdlog