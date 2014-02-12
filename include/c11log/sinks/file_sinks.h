#pragma once

#include <fstream>
#include  <iomanip>
#include <mutex>

#include "base_sink.h"

namespace c11log
{
namespace sinks
{
/*
* Trivial file sink with single file as target
*/
class simple_file_sink : public base_sink
{
public:
	explicit simple_file_sink(const std::string &filename, const std::string& extension = "txt")
		: mutex_(),
		  _ofstream(filename + "." + extension, std::ofstream::app)
	{}
protected:
	void sink_it_(const std::string& msg) override {
		std::lock_guard<std::mutex> lock(mutex_);
		_ofstream << msg;
		_ofstream.flush();
	}
private:
	std::mutex mutex_;
	std::ofstream _ofstream;
};



/*
 * Thread safe, size limited file sink
*/
class rotating_file_sink : public base_sink
{
public:
	rotating_file_sink(const std::string &base_filename, const std::string &extension, size_t max_size, size_t max_files):
		_base_filename(base_filename),
		_extension(extension),
		_max_size(max_size),
		_max_files(max_files),
		_current_size(0),
		mutex_(),
		_ofstream(_calc_filename(_base_filename, 0, _extension))
	{}

protected:
	void sink_it_(const std::string& msg) override {
		std::lock_guard<std::mutex> lock(mutex_);
		_current_size += msg.length();
		if (_current_size  > _max_size) {
			_rotate();
			_current_size = msg.length();
		}
		_ofstream << msg;
		_ofstream.flush();
	}


private:
	static std::string _calc_filename(const std::string& filename, std::size_t index, const std::string& extension) {
		std::ostringstream oss;
		if (index)
			oss << filename << "." << index << "." << extension;
		else
			oss << filename << "." << extension;
		return oss.str();
	}


	// Rotate old files:
	// log.n-1.txt -> log.n.txt
	// log n-2.txt -> log.n-1.txt
	// ...
	// log.txt -> log.1.txt
	void _rotate() {
		_ofstream.close();
		//Remove oldest file
		for (auto i = _max_files; i > 0; --i) {
			auto src = _calc_filename(_base_filename, i - 1, _extension);
			auto target = _calc_filename(_base_filename, i, _extension);
			if (i == _max_files)
				std::remove(target.c_str());
			std::rename(src.c_str(), target.c_str());
		}
		_ofstream.open(_calc_filename(_base_filename, 0, _extension));
	}

	std::string _base_filename;
	std::string _extension;
	std::size_t _max_size;
	std::size_t _max_files;
	std::size_t _current_size;
	std::mutex mutex_;
	std::ofstream _ofstream;

};

/*
 * Thread safe file sink that closes the log file at midnight and opens new one
 */
class daily_file_sink:public base_sink
{
public:
	explicit daily_file_sink(const std::string& base_filename, const std::string& extension = "txt"):
		_base_filename(base_filename),
		_extension(extension),
		_midnight_tp (_calc_midnight_tp() ),
		mutex_(),
		_ofstream(_calc_filename(_base_filename, _extension), std::ofstream::app)
	{}

protected:
	void sink_it_(const std::string& msg) override {
		std::lock_guard<std::mutex> lock(mutex_);
		if (std::chrono::system_clock::now() >= _midnight_tp) {
			_ofstream.close();
			_ofstream.open(_calc_filename(_base_filename, _extension));
			_midnight_tp = _calc_midnight_tp();
		}
		_ofstream << msg;
		_ofstream.flush();
	}

private:
	// Return next midnight's time_point
	static std::chrono::system_clock::time_point _calc_midnight_tp() {
		using namespace std::chrono;
		auto now = system_clock::now();
		time_t tnow = std::chrono::system_clock::to_time_t(now);
		tm date = c11log::details::os::localtime(tnow);
		date.tm_hour = date.tm_min = date.tm_sec = 0;
		auto midnight = std::chrono::system_clock::from_time_t(std::mktime(&date));
		return system_clock::time_point(midnight + hours(24));
	}

	static std::string _calc_filename(const std::string& basename, const std::string& extension) {
		std::tm tm = c11log::details::os::localtime();
		char buf[32];
		sprintf(buf, ".%d-%02d-%02d.", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		return basename+buf+extension;
	}

	std::string _base_filename;
	std::string _extension;
	std::chrono::system_clock::time_point _midnight_tp;
	std::mutex mutex_;
	std::ofstream _ofstream;

};
}
}
