#pragma once
#include "spdlog\sinks\sink.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/common.h"
#include <sqlite3.h>

namespace spdlog
{
	namespace sinks
	{
		class database_logger_sink :
			public sink
		{
		public:

			explicit database_logger_sink(const std::string& databaseName)
			{
				if (sqlite3_open(databaseName.c_str(), &_database))
					throw spdlog_ex("Error opening database");
			}

			~database_logger_sink()
			{
				sqlite3_close(_database);
			}

			void flush() override
			{
				sqlite3_close(_database);
			}

			sqlite3_stmt * prepare_query(const details::log_msg& msg) const
			{
				auto time = std::chrono::system_clock::to_time_t(msg.time);

				char time_str[26];

				ctime_s(time_str, sizeof(time_str), &time);

				sqlite3_stmt * query_stmt;

				if (sqlite3_prepare_v2(_database, "INSERT INTO Logs (TimeStamp,Level,Message,LoggerName,ThreadId) VALUES (?,?,?,?,?)", -1, &query_stmt, nullptr) != SQLITE_OK)
					throw spdlog_ex(sqlite3_errmsg(_database));

				if (sqlite3_bind_text(query_stmt, 1, time_str, -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_text(query_stmt, 2, to_str(msg.level), -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_text(query_stmt, 3, msg.raw.c_str(), -1, nullptr) != SQLITE_OK ||
					sqlite3_bind_text(query_stmt, 4, "'''''''''''", -1, SQLITE_STATIC) != SQLITE_OK ||
					sqlite3_bind_int(query_stmt, 5, msg.thread_id) != SQLITE_OK)
					throw spdlog_ex(sqlite3_errmsg(_database));

				return query_stmt;
			}

			void log(const details::log_msg& msg) override
			{
				auto query_stmt = prepare_query(msg);

				if (sqlite3_step(query_stmt) != SQLITE_DONE)
				{
					throw spdlog_ex(sqlite3_errmsg(_database));
				}

				sqlite3_finalize(query_stmt);
			}

		private:
			sqlite3 *_database;
		};
	}
}
