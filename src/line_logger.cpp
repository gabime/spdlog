#include "stdafx.h"
#include "c11log/logger.h"

c11log::details::line_logger::line_logger(logger* callback_logger, level::level_enum msg_level) :
    _callback_logger(callback_logger)
{
    if (callback_logger) {
        callback_logger->_formatter->format_header(callback_logger->_logger_name,
                msg_level,
                c11log::formatters::timepoint::clock::now(),
                _oss);
    }
}

c11log::details::line_logger::~line_logger()
{
    if (_callback_logger) {
        _oss << '\n';
        _callback_logger->_log_it(_oss.str_ref());
    }
}