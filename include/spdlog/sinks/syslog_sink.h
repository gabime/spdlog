/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#ifdef __linux__

#include <string>
#include <syslog.h>
#include "./sink.h"
#include "../common.h"
#include "../details/log_msg.h"


namespace spdlog
{
  namespace sinks
  {
    /**
     * Sink that write to syslog using the `syscall()` library call.
     *
     * Locking is not needed, as `syslog()` itself is thread-safe.
     */
    class syslog_sink : public sink
    {
    public:
      syslog_sink()
	{
	  _priorities[static_cast<int>(level::TRACE)] = LOG_DEBUG;
	  _priorities[static_cast<int>(level::DEBUG)] = LOG_DEBUG;
	  _priorities[static_cast<int>(level::INFO)] = LOG_INFO;
	  _priorities[static_cast<int>(level::NOTICE)] = LOG_NOTICE;
	  _priorities[static_cast<int>(level::WARN)] = LOG_WARNING;
	  _priorities[static_cast<int>(level::ERR)] = LOG_ERR;
	  _priorities[static_cast<int>(level::CRITICAL)] = LOG_CRIT;
	  _priorities[static_cast<int>(level::ALERT)] = LOG_ALERT;
	  _priorities[static_cast<int>(level::EMERG)] = LOG_EMERG;

	  _priorities[static_cast<int>(level::ALWAYS)] = LOG_INFO;
	  _priorities[static_cast<int>(level::OFF)] = LOG_INFO;
	}
      virtual ~syslog_sink() = default;
  
      syslog_sink(const syslog_sink&) = delete;
      syslog_sink& operator=(const syslog_sink&) = delete;
  
      void log(const details::log_msg &msg) override
      {
	syslog(syslog_prio_from_level(msg), "%s", msg.formatted.str().c_str());
      };

    protected:
      /**
       * Simply maps spdlog's log level to syslog priority level.
       */
      int syslog_prio_from_level(const details::log_msg &msg) const
      {
	return _priorities[static_cast<int>(msg.level)];
      }

    private:
      std::array<int, 11> _priorities;
    };
  }
}

#endif
