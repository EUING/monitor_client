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

		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(L"CREATE TABLE IF NOT EXISTS items(id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, parent_id INTEGER NOT NULL, name TEXT NOT NULL, size INTERGER NOT NULL, hash TEXT NOT NULL, UNIQUE(parent_id, name));");
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

	std::optional<common_utility::ItemInfo> ItemDaoSqlite::GetItemInfo(const std::wstring& item_name, int parent_id) const {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::GetFileInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT name, size, hash FROM items WHERE name='%s' and parent_id=%d;", item_name.c_str(), parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			std::wcerr << L"ItemDaoSqlite::GetItemInfo: ExecuteQuery Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		auto row = result.value();
		if (row.empty()) {
			std::wcerr << L"ItemDaoSqlite::GetItemInfo: Can't find item: " << query << std::endl;
			return std::nullopt;
		}

		common_utility::ItemInfo info;
		info.name = row[0][L"name"];
		info.size = row[0][L"size"].empty() ? -1 : _wtoi(row[0][L"size"].c_str());
		info.hash = row[0][L"hash"];

		return info;
	}

	std::optional<std::vector<common_utility::ItemInfo>> ItemDaoSqlite::GetFolderContainList(int parent_id) const {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::GetFolderContainList: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT name, size, hash FROM items WHERE parent_id=%d;", parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			std::wcerr << L"ItemDaoSqlite::GetFolderContainList: ExecuteQuery Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		std::vector<common_utility::ItemInfo> v;
		auto row = result.value();

		for (const auto& data : row) {
			std::wstring name = data.at(L"name");
			std::wstring size = data.at(L"size");
			std::wstring hash = data.at(L"hash");

			common_utility::ItemInfo info;
			info.name = name;
			info.size = size.empty() ? -1 : _wtoi(size.c_str());
			info.hash = hash;

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

	std::optional<int> ItemDaoSqlite::InsertItemInfo(const common_utility::ItemInfo& item_info, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::InsertItemInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"INSERT INTO items(size, hash, name, parent_id) VALUES(%d, '%s', '%s', %d);", item_info.size, item_info.hash.c_str(), item_info.name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::InsertItemInfo: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}

	std::optional<int> ItemDaoSqlite::ModifyItemInfo(const common_utility::ItemInfo& item_info, int parent_id) {
		if (!sqlite_wrapper_) {
			std::wcerr << L"ItemDaoSqlite::ModifyItemInfo: sqlite_wrapper_ is null" << std::endl;
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"UPDATE items SET size=%d, hash='%s' WHERE name='%s' and parent_id=%d;", item_info.size, item_info.hash.c_str(), item_info.name.c_str(), parent_id);
		sqlite_manager::SqlError error = sqlite_wrapper_->ExecuteUpdate(query);
		if (sqlite_manager::SqlError::SQLITE_OK != error) {
			std::wcerr << L"ItemDaoSqlite::ModifyItemInfo: ExecuteUpdate Fail: " << query << L' ' << sqlite_wrapper_->GetLastError() << std::endl;
			return std::nullopt;
		}

		return sqlite_wrapper_->GetLastChangeRowCount();
	}
}  // namespace monitor_client