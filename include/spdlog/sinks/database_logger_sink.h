#pragma once
#include <spdlog\sinks\sink.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/common.h>
#include <vector>
#include <sqlite3.h>

struct Column
{
	enum TableColumn
	{
		TimeStamp,
		Level,
		Message,
		LoggerName,
		ThreadId
	};

	Column(std::string columnName, TableColumn columnMap)
	{
		ColumnName = columnName;
		ColumnMap = columnMap;
	}

	std::string ColumnName;
	TableColumn ColumnMap;
	std::string Value;
};

struct database_schema
{
	std::string TableName;

	std::vector<Column> Columns;

	database_schema(const std::string& tableName, const std::vector<Column>& columns)
	{
		TableName = tableName;
		Columns = columns;
	}

	database_schema()
	{
		TableName = "Logs";

		Columns =
		{
			Column("TimeStamp", Column::TimeStamp),
			Column("Level", Column::Level),
			Column("Message", Column::Message),
			Column("LoggerName", Column::LoggerName),
			Column("ThreadId", Column::ThreadId)
		};
	}
};

namespace spdlog
{
	namespace sinks
	{
		class database_logger_sink :
			public sink
		{
		public:
			void flush() override
			{
				sqlite3_close(_database);
			}

			void log(const details::log_msg& msg) override
			{

				for (auto& column : _schema.Columns)
				{
					switch (column.ColumnMap)
					{
					case Column::TimeStamp:
					{
						auto time = std::chrono::system_clock::to_time_t(msg.time);
						char str[26];
						ctime_s(str, sizeof(str), &time);
						column.Value = str;
						break;
					}

					case Column::Level:
					{
						column.Value = level::to_str(msg.level);
						break;
					}
					case Column::Message:
					{
						column.Value = msg.raw.str();
						break;
					}
					case Column::LoggerName:
					{
						column.Value = msg.logger_name;
						break;
					}
					case Column::ThreadId:
					{
						column.Value = std::to_string(msg.thread_id);
						break;
					}
					}
				}

				auto query = fmt::format("INSERT INTO {0} ({1},{3},{5},{7},{9}) VALUES ('{2}','{4}','{6}','{8}',{10})",
					_schema.TableName,
					_schema.Columns[0].ColumnName,
					_schema.Columns[0].Value,
					_schema.Columns[1].ColumnName,
					_schema.Columns[1].Value,
					_schema.Columns[2].ColumnName,
					_schema.Columns[2].Value,
					_schema.Columns[3].ColumnName,
					_schema.Columns[3].Value,
					_schema.Columns[4].ColumnName,
					_schema.Columns[4].Value);

				char *errorMessage = nullptr;

				if (sqlite3_exec(_database, query.c_str(), nullptr, nullptr, &errorMessage) != SQLITE_OK)
				{
					throw spdlog_ex(errorMessage);
				}


			}

			explicit database_logger_sink(const std::string& databaseName)
			{
				if (sqlite3_open(databaseName.c_str(), &_database))
					throw spdlog_ex("Error opening database");
			}

			explicit database_logger_sink(const std::string& databaseName, const database_schema& databaseSchema)
			{
				_schema = databaseSchema;

				if (sqlite3_open(databaseName.c_str(), &_database))
					throw spdlog_ex("Error opening database");
			}

			~database_logger_sink()
			{
				sqlite3_close(_database);
			}

		private:
			database_schema _schema;
			sqlite3 *_database;
		};
	}
}
