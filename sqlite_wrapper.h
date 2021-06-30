#ifndef SQLITE_MANAGER_SQLITE_WRAPPER_H_
#define SQLITE_MANAGER_SQLITE_WRAPPER_H_

#ifdef SQLITEMANAGER_EXPORTS
#define SQLITE_DECLSPEC __declspec(dllexport)
#else
#define SQLITE_DECLSPEC __declspec(dllimport)
#endif

#include <optional>
#include <vector>
#include <string>
#include <map>
#include <memory>

struct sqlite3;

namespace sqlite_manager {
namespace utf8 {
	using DataSet = std::map<std::string, std::string>;

	class SQLITE_DECLSPEC SqliteWrapper {
	public:
		static std::unique_ptr<SqliteWrapper> Create(const std::string& database_path);

		SqliteWrapper(const SqliteWrapper&) = delete;
		SqliteWrapper& operator=(const SqliteWrapper&) = delete;

		SqliteWrapper(SqliteWrapper&&) = delete;
		SqliteWrapper& operator=(SqliteWrapper&&) = delete;

		~SqliteWrapper();

		int ExecuteUpdate(const std::string& query);
		std::optional<std::vector<DataSet>> ExecuteQuery(const std::string& query);

	private:
		SqliteWrapper() = default;

		sqlite3* sqlite_;
	};
}  // namespace utf8

namespace utf16 {
	using DataSet = std::map<std::wstring, std::wstring>;

	class SQLITE_DECLSPEC SqliteWrapper {
	public:
		static std::unique_ptr<SqliteWrapper> Create(const std::wstring& database_path);

		SqliteWrapper(const SqliteWrapper&) = delete;
		SqliteWrapper& operator=(const SqliteWrapper&) = delete;

		SqliteWrapper(SqliteWrapper&&) = delete;
		SqliteWrapper& operator=(SqliteWrapper&&) = delete;

		~SqliteWrapper();

		int ExecuteUpdate(const std::wstring& query);
		std::optional<std::vector<DataSet>> ExecuteQuery(const std::wstring& query);

	private:
		SqliteWrapper() = default;

		sqlite3* sqlite_;
	};
}  // namespace utf16
}  // namespace sqlite_manager
#endif