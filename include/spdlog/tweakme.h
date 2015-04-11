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

///////////////////////////////////////////////////////////////////////////////
// Edit this file to squeeze every last drop of performance out of spdlog.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Under Linux, the much faster CLOCK_REALTIME_COARSE clock can be used.
// This clock is less accurate - can be off by dozens of millis - depending on the kernel HZ
// Uncomment to use it instead of the regular (but slower) clock.
// #define SPDLOG_CLOCK_COARSE
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Uncomment if date/time logging is not needed.
// This will prevent spdlog from quering the clock on each log call.
// #define SPDLOG_NO_DATETIME
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Uncomment if thread id logging is not needed (i.e. no %t in the log pattern)
// This will prevent spdlog from quering the thread id on each log call.
// #define SPDLOG_NO_THREAD_ID
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Uncomment if logger name logging is not needed.
// This will prevent spdlog from copying the logger name  on each log call.
// #define SPDLOG_NO_NAME
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Uncomment to enable the SPDLOG_DEBUG/SPDLOG_TRACE macros
// #define SPDLOG_DEBUG_ON
// #define SPDLOG_TRACE_ON
///////////////////////////////////////////////////////////////////////////////
