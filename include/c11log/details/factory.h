#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>


namespace c11log
{
namespace details
{
class factory
{
public:
    using logger_ptr = std::shared_ptr<c11log::logger>;
    using logger_map = std::unordered_map<std::string, logger_ptr>;
    std::shared_ptr<logger> create_logger(const std::string& name, logger::sinks_init_list, logger::formatter_ptr = nullptr);
    logger_ptr get_logger(const std::string &name);
    static factory& instance();

private:
    std::mutex _factory_mutex;
    logger_map _factory;



};
}
}


inline std::shared_ptr<c11log::logger> c11log::details::factory::create_logger(const std::string& name, logger::sinks_init_list sinks, logger::formatter_ptr formatter)
{
    std::lock_guard<std::mutex> lock(_factory_mutex);
    logger_ptr logger_p = std::make_shared<logger>(name, sinks, std::move(formatter));

    _factory.insert(logger_map::value_type(name, logger_p));
    return logger_p;
}



inline c11log::details::factory::logger_ptr c11log::details::factory::get_logger(const std::string &name)
{
    std::lock_guard<std::mutex> lock(_factory_mutex);

    auto found = _factory.find(name);
    if (found != _factory.end())
        return found->second;
    else
        return logger_ptr(nullptr);
}


inline c11log::details::factory & c11log::details::factory::instance()
{
    static factory instance;
    return instance;
}
