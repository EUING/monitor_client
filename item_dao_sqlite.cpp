#include "item_dao_sqlite.h"

#include <iostream>
#include <optional>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "sqlite_wrapper.h"

#include "common_utility.h"

namespace monitor_client {
	bool ItemDaoSqlite::OpenDatabase(const std::wstring& database_path) {
		sqlite_wrapper_ = sqlite_manager::utf16::SqliteWrapper::Create(database_path);
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::OpenDatabase: Create Fail: " << database_path << std::endl;
			return false;
		}

		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(L"CREATE TABLE IF NOT EXISTS items(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, parent_id INTEGER NOT NULL, name TEXT NOT NULL, size INTERGER, hash TEXT, UNIQUE(parent_id, name));");
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::OpenDatabase: ExecuteUpdate Fail: " << sqlite_wrapper_->GetLastError() << std::endl;
			return false;
		}

		return true;
	}

	std::optional<int> ItemDaoSqlite::GetItemId(const std::wstring& item_name, int parent_id) const {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::GetItemId: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT id FROM items WHERE name='%s' and parent_id=%d;", item_name.c_str(), parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			std::wcerr << L"ItemDaoSqlite::GetItemId: ExecuteQuery Fail: " << query  << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		auto row = result.value();
		if (row.empty()) {
			std::wcerr << L"ItemDaoSqlite::GetItemId: Can't find item: " << query << std::endl;
			return std::nullopt;
		}

		return _wtoi(row[0][L"id"].c_str());
	}

	std::optional<common_utility::FileInfo> ItemDaoSqlite::GetFileInfo(const std::wstring& file_name, int parent_id) const {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::GetFileInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT name, size FROM items WHERE name='%s' and parent_id=%d;", file_name.c_str(), parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			std::wcerr << L"ItemDaoSqlite::GetItemId: ExecuteQuery Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		auto row = result.value();
		if (row.empty()) {
			std::wcerr << L"ItemDaoSqlite::GetItemId: Can't find item: " << query << std::endl;
			return std::nullopt;
		}

		common_utility::FileInfo info;
		info.name = row[0][L"name"];
		info.size = row[0][L"size"].empty() ? -1 : _wtoi(row[0][L"size"].c_str());

		return info;
	}

	std::optional<std::vector<common_utility::FileInfo>> ItemDaoSqlite::GetFolderContainList(int parent_id) const {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::GetFolderContainList: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT name, size FROM items WHERE parent_id=%d;", parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			std::wcerr << L"ItemDaoSqlite::GetItemId: ExecuteQuery Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		std::vector<common_utility::FileInfo> v;
		auto row = result.value();

		for (const auto& data : row) {
			std::wstring name = data.at(L"name");
			std::wstring size = data.at(L"size");

			common_utility::FileInfo info;
			info.name = name;
			info.size = size.empty() ? -1 : _wtoi(size.c_str());

			v.push_back(info);
		}

		return v;
	}

	std::optional<int> ItemDaoSqlite::ChangeItemName(const common_utility::ChangeNameInfo& name_info, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::ChangeItemName: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"UPDATE items SET name='%s' WHERE name='%s' and parent_id=%d;", name_info.new_name.c_str(), name_info.old_name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::ChangeItemName: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}

	std::optional<int> ItemDaoSqlite::DeleteItemInfo(const std::wstring& item_name, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::DeleteItemInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"DELETE FROM items WHERE name='%s' and parent_id=%d;", item_name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::DeleteItemInfo: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}

	std::optional<int> ItemDaoSqlite::InsertFileInfo(const common_utility::FileInfo& file_info, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::InsertFileInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"INSERT INTO items(size, name, parent_id) VALUES(%d, '%s', %d);", file_info.size, file_info.name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::InsertFileInfo: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}

	std::optional<int> ItemDaoSqlite::ModifyFileInfo(const common_utility::FileInfo& file_info, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::ModifyFileInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"UPDATE items SET size=%d WHERE name='%s' and parent_id=%d;", file_info.size, file_info.name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::ModifyFileInfo: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}

	std::optional<int> ItemDaoSqlite::InsertFolderInfo(const common_utility::FolderInfo& folder_info, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::InsertFolderInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"INSERT INTO items(name, parent_id) VALUES('%s', %d);", folder_info.name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::InsertFolderInfo: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}
}  // namespace monitor_client