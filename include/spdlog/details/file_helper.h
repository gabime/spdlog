//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

// Helper class for file sink
// When failing to open a file, retry several times(5) with small delay between the tries(10 ms)
// Can be set to auto flush on every line
// Throw spdlog_ex exception on errors

#include <string>
#include <thread>
#include <chrono>
#include "os.h"
#include "log_msg.h"



namespace spdlog
{
namespace details
{

class file_helper
{
public:
    const int open_tries = 5;
    const int open_interval = 10;

    explicit file_helper(bool force_flush) :
        _fd(nullptr),
        _force_flush(force_flush)
    {}

    file_helper(const file_helper&) = delete;
    file_helper& operator=(const file_helper&) = delete;

    ~file_helper()
    {
        close();
    }


    void open(const std::string& fname, bool truncate = false)
    {

        close();
        const char* mode = truncate ? "wb" : "ab";
        _filename = fname;
        for (int tries = 0; tries < open_tries; ++tries)
        {
            if(!_dir_created) 
            {
                if (dir_check(fname)) 
                {
                    get_dirlist(fname);
                    if(create_dirs())
                        break;
                }
            }

            if(!os::fopen_s(&_fd, fname, mode))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(open_interval));
        }

        throw spdlog_ex("Failed opening file " + fname + " for writing");
    }

    void reopen(bool truncate)
    {
        if (_filename.empty())
            throw spdlog_ex("Failed re opening file - was not opened before");
        open(_filename, truncate);

    }

    void flush()
    {
        std::fflush(_fd);
    }

    void close()
    {
        if (_fd)
        {
            std::fclose(_fd);
            _fd = nullptr;
        }
    }

    void write(const log_msg& msg)
    {

        size_t msg_size = msg.formatted.size();
        auto data = msg.formatted.data();
        if (std::fwrite(data, 1, msg_size, _fd) != msg_size)
            throw spdlog_ex("Failed writing to file " + _filename);

        if (_force_flush)
            std::fflush(_fd);

    }

    long size()
    {
        if (!_fd)
            throw spdlog_ex("Cannot use size() on closed file " + _filename);

        auto pos = ftell(_fd);
        if (fseek(_fd, 0, SEEK_END) != 0)
            throw spdlog_ex("fseek failed on file " + _filename);

        auto file_size = ftell(_fd);

        if(fseek(_fd, pos, SEEK_SET) !=0)
            throw spdlog_ex("fseek failed on file " + _filename);

        if (file_size == -1)
            throw spdlog_ex("ftell failed on file " + _filename);


        return file_size;


    }

    const std::string& filename() const
    {
        return _filename;
    }

    static bool file_exists(const std::string& name)
    {

        return os::file_exists(name);
    }

    //Return true if filenamd contains relative path
    bool dir_check(const std::string &name)
    {
        std::string::size_type index = 0;

        index = name.find("/", index);
        if(index != std::string::npos) 
        {
            if(index == 0)
                _path_type = 1;
            else
                _path_type = 0;

            return true;
        }
        
        return false;
    }

    //Get list contain directories name 
    void get_dirlist(const std::string &name)
    {
        std::string directory;
        std::string::size_type index, previndex = 0;

        index = name.find("/", previndex);

        do {
            if(index - previndex == 0)
                directory = name.substr(previndex, index - previndex + 1);
            else
                directory = name.substr(previndex, index - previndex);

            if(directory != "/")
                dirs.push_back(directory);

            previndex = index + 1;
            index = name.find("/", previndex);
        }while(index != std::string::npos);
    }


    std::string make_dirstring(int depth)
    {
        std::string dirname;
        std::list<std::string>::iterator li;

        int i;

        if(_path_type)
            dirname = "/";

        for(i = 0, li = dirs.begin(); i <= depth; i++, li++) 
            dirname = dirname + *li + "/";

        return dirname;
    }


    //Create directories by referring to list
    //Return 0 if success
    bool create_dirs() 
    {
        std::string dirname;
        std::list<std::string>::iterator li;

        int r, depth = 0;

        for(li = dirs.begin(); li != dirs.end(); li++, depth++)
        { 
            dirname = make_dirstring(depth);
            r = os::_mkdir(dirname);

            if(r)
            {
#ifdef _WIN32
                //not yet
#else
                if(os::get_lasterror() != EEXIST)
                    return r;
#endif
            }
        }

        _dir_created = 1;

        return 0;
    }


private:
    FILE* _fd;
    std::string _filename;
    std::list<std::string> dirs;
    bool _dir_created;
    bool _path_type;
    bool _force_flush;


};
}
}
