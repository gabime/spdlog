#pragma once
#include <fstream>
#include  <iomanip>

#include "../logger.h"
#include "../log_exception.h"
#include "../details/os.h"

#include "base_sink.h"

namespace c11log {
namespace sinks {
/*
* Trivial file sink with single file as target
*/
class simple_file_sink:base_sink {
public:
    simple_file_sink(const std::string &filename, const std::string& extension = "txt")
    {
        std::ostringstream oss;
        oss << filename << "." << extension;
        _ofstream.open(oss.str(), std::ofstream::app);
    }
protected:
    void _sink_it(const std::string& msg) override
    {
        _ofstream << msg;
        _ofstream.flush();
    }

    std::ofstream _ofstream;
};


/*
 * Rotating file sinks. Close and open new file at some point
 */
namespace details {
class rotating_file_sink_base:public base_sink {
public:
    rotating_file_sink_base()
    {}
    virtual ~rotating_file_sink_base()
    {}
protected:
    virtual void _sink_it(const std::string& msg) override
    {
        if (_should_rotate())
            _rotate();
        _ofstream << msg;
        _ofstream.flush();
    }
    virtual bool _should_rotate() const = 0;
    virtual void _rotate() = 0;
    std::ofstream _ofstream;
};
}
class rotating_file_sink:public details::rotating_file_sink_base {
public:
    rotating_file_sink(const std::string &base_filename, const std::string &extension, size_t max_size, size_t max_files):
        _base_filename(base_filename),
        _extension(extension),
        _max_size(max_size),
        _max_files(max_files),
        _current_size(0),
        _index(0)
    {
        _ofstream.open(_calc_filename(_base_filename, 0, _extension));
    }

protected:
    virtual void _sink_it(const std::string& msg) override
    {
        _current_size += msg.length();
        rotating_file_sink_base::_sink_it(msg);
    }

    bool _should_rotate() const override
    {
        return _current_size >= _max_size;
    }

    // Rotate old files:
    // log.n-1.txt -> log.n.txt
    // log n-2.txt -> log.n-1.txt
    // ...
    // log.txt -> log.1.txt
    void _rotate() override
    {
        _ofstream.close();
        _current_size = 0;
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
private:
    static std::string _calc_filename(const std::string& filename, std::size_t index, const std::string& extension)
    {
        std::ostringstream oss;
        if (index)
            oss << filename << "." << index << "." << extension;
        else
            oss << filename << "." << extension;
        return oss.str();
    }
    std::string _base_filename;
    std::string _extension;
    std::size_t _max_size;
    std::size_t _max_files;
    std::size_t _current_size;
    std::size_t _index;

};

/*
 * File sink that closes the log file at midnight and opens new one
 */
class midnight_file_sink:public details::rotating_file_sink_base {
public:
    midnight_file_sink(const std::string& base_filename, const std::string& extension):
        _base_filename(base_filename),
        _extension(extension),
        _midnight_tp { _calc_midnight_tp() }

    {
        _ofstream.open(_calc_filename(_base_filename, _extension));
    }

protected:
    bool _should_rotate() const override
    {
        return std::chrono::system_clock::now() >= _midnight_tp;
    }
    void _rotate() override
    {
        _midnight_tp = _calc_midnight_tp();
        _ofstream.close();
        _ofstream.open(_calc_filename(_base_filename, _extension));

    }

private:
    // Return next midnight's time_point
    static std::chrono::system_clock::time_point _calc_midnight_tp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        time_t tnow = std::chrono::system_clock::to_time_t(now);
        tm date = c11log::details::os::localtime(tnow);
        date.tm_hour = date.tm_min = date.tm_sec = 0;
        auto midnight = std::chrono::system_clock::from_time_t(std::mktime(&date));
        return system_clock::time_point(midnight + hours(24));
    }

    static std::string _calc_filename(const std::string& basename, const std::string& extension)
    {
        std::ostringstream oss;
        std::tm now_tm = c11log::details::os::localtime();
        oss << basename << std::put_time(&now_tm, ".%Y-%m-%d.") << extension;
        return oss.str();
    }
    std::string _base_filename;
    std::string _extension;
    std::chrono::system_clock::time_point _midnight_tp;
};
}
}