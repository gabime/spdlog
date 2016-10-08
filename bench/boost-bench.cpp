//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void init()
{
    logging::add_file_log
    (
        keywords::file_name = "logs/boost-sample_%N.log",                              /*< file name pattern >*/
        keywords::auto_flush = false,
        keywords::format = "[%TimeStamp%]: %Message%"
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}


int main(int argc, char* [])
{
    int howmany = 1000000;
    init();
    logging::add_common_attributes();

    using namespace logging::trivial;
    src::severity_logger_mt< severity_level > lg;
    for(int i  = 0 ; i < howmany; ++i)
        BOOST_LOG_SEV(lg, info) << "boost message #" << i << ": This is some text for your pleasure";

    return 0;
}
