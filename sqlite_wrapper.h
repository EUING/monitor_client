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
	enum class SQLITE_DECLSPEC SqlError {
		SQLITE_OK,            // Successful result
		SQLITE_ERROR,         // Generic error
		SQLITE_INTERNAL,      // Internal logic error in SQLite
		SQLITE_PERM,          // Access permission denied
		SQLITE_ABORT,         // Callback routine requested an abort
		SQLITE_BUSY,          // The database file is locked
		SQLITE_LOCKED,        // A table in the database is locked
		SQLITE_NOMEM,         // A malloc() failed
		SQLITE_READONLY,      // Attempt to write a readonly database
		SQLITE_INTERRUPT,     // Operation terminated by sqlite3_interrupt()
		SQLITE_IOERR,         // Some kind of disk I/O error occurred
		SQLITE_CORRUPT,       // The database disk image is malformed
		SQLITE_NOTFOUND,      // Unknown opcode in sqlite3_file_control()
		SQLITE_FULL,          // Insertion failed because database is full
		SQLITE_CANTOPEN,      // Unable to open the database file
		SQLITE_PROTOCOL,      // Database lock protocol error
		SQLITE_EMPTY,         // Internal use only
		SQLITE_SCHEMA,        // The database schema changed
		SQLITE_TOOBIG,        // String or BLOB exceeds size limit
		SQLITE_CONSTRAINT,    // Abort due to constraint violation
		SQLITE_MISMATCH,      // Data type mismatch
		SQLITE_MISUSE,        // Library used incorrectly
		SQLITE_NOLFS,         // Uses OS features not supported on host
		SQLITE_AUTH,          // Authorization denied
		SQLITE_FORMAT,        // Not used
		SQLITE_RANGE,         // 2nd parameter to sqlite3_bind out of range
		SQLITE_NOTADB,        // File opened that is not a database file
		SQLITE_NOTICE,        // Notifications from sqlite3_log()
		SQLITE_WARNING,       // Warnings from sqlite3_log()
		SQLITE_ROW = 100,     // sqlite3_step() has another row ready
		SQLITE_DONE           // sqlite3_step() has finished executing
	};

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

		SqlError ExecuteUpdate(const std::string& query);
		std::optional<std::vector<DataSet>> ExecuteQuery(const std::string& query);
		std::optional<int> GetLastChangeRowCount() const;
		std::string GetLastError() const { return last_error_; }

	private:
		SqliteWrapper() = default;

		std::string last_error_;
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

		SqlError ExecuteUpdate(const std::wstring& query);
		std::optional<std::vector<DataSet>> ExecuteQuery(const std::wstring& query);
		std::optional<int> GetLastChangeRowCount() const;
		std::wstring GetLastError() const { return last_error_; }

	private:
		SqliteWrapper() = default;

		std::wstring last_error_;
		sqlite3* sqlite_;
	};
}  // namespace utf16
}  // namespace sqlite_manager
#endif