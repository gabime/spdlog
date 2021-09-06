// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

namespace spdlog {
namespace details {

// https://datatracker.ietf.org/doc/html/rfc5424#section-6.2.1
struct syslog_consts {
    enum level {
        SL_EMERG = 0,
        SL_ALERT,
        SL_CRIT,
        SL_ERR,
        SL_WARNING,
        SL_NOTICE,
        SL_INFO,
        SL_DEBUG
    };

    enum facility {
        SF_KERN = 0,
        SF_USER,
        SF_MAIL,
        SF_DAEMON,
        SF_AUTH,
        SF_SYSLOG,
        SF_LPR,
        SF_NEWS,
        SF_UUCP,
        SF_CRON,
        SF_AUTHPRIV,
        SF_FTP,
        SF_NTP,
        SF_AUDIT,
        SF_ALERT,
        SF_CRON2,
        SF_LOCAL0,
        SF_LOCAL1,
        SF_LOCAL2,
        SF_LOCAL3,
        SF_LOCAL4,
        SF_LOCAL5,
        SF_LOCAL6,
        SF_LOCAL7
    };
};

} // namespace details
} // namespace spdlog