#pragma once
#include "spdloglb.h"


namespace spdlog
{

#ifdef __cplusplus
	extern "C" {
#endif


	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateOstreamSink(spdlog::std_ostream_t& os, bool ForceFlush, bool MultiThreaded);

	
	SPDLOGLB_API spdlog::log_handle_t SpdLog_CreateOstreamLogger(const char* LoggerName,
		spdlog::std_ostream_t& os, 
		bool ForceFlush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

	
	SPDLOGLB_API spdlog::log_handle_t SpdLog_GetOrCreateOstreamLogger(const char* LoggerName,
		spdlog::std_ostream_t& os,
		bool ForceFlush,
		bool MultiThreaded,
		SPDLOGLB_CREATELOGGER_PARAM* lp_prop);

#ifdef __cplusplus
	}
#endif



} // namespace spdlog