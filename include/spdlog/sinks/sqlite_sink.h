#pragma once
#include "spdlog\sinks\sink.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/common.h"
#include <sqlite3.h>

namespace spdlog
{
	namespace sinks
	{
		class sqlite_sink :
			public sink
		{
		public:

			explicit sqlite_sink(const std::string& databaseName)
			{
				if (sqlite3_open(databaseName.c_str(), &_database))
					throw spdlog_ex("Error opening database");

				if (sqlite3_prepare_v2(_database, "INSERT INTO Logs (TimeStamp,Level,Message,LoggerName,ThreadId) VALUES (?,?,?,?,?)", -1, &_query_stmt, nullptr) != SQLITE_OK)
					throw spdlog_ex(sqlite3_errmsg(_database));
			}

			~sqlite_sink()
			{
				sqlite_sink::flush();
			}

			void flush() override
			{
				sqlite3_close(_database);

				sqlite3_finalize(_query_stmt);

				_database = nullptr;
				_query_stmt = nullptr;
			}

			void bind_to_statement(const details::log_msg& msg) const
			{
				auto time = std::chrono::system_clock::to_time_t(msg.time);

				char time_str[26];

				ctime_s(time_str, sizeof(time_str), &time);

				if (sqlite3_bind_text(_query_stmt, 1, time_str, -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_text(_query_stmt, 2, to_str(msg.level), -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_text(_query_stmt, 3, msg.raw.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_text(_query_stmt, 4, msg.logger_name.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_int(_query_stmt, 5, msg.thread_id) != SQLITE_OK)
					throw spdlog_ex(sqlite3_errmsg(_database));
			}

			void log(const details::log_msg& msg) override
			{
				bind_to_statement(msg);

				if (sqlite3_step(_query_stmt) != SQLITE_DONE)
				{
					throw spdlog_ex(sqlite3_errmsg(_database));
				}

				sqlite3_reset(_query_stmt);
				sqlite3_clear_bindings(_query_stmt);
			}

		private:
			sqlite3 *_database;

			sqlite3_stmt * _query_stmt;
		};
	}
}
