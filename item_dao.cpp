#include "item_dao.h"

#include <optional>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "common_utility.h"

namespace monitor_client {
	bool ItemDao::OpenDatabase(const std::wstring& database_path) {
		sqlite_wrapper_ = sqlite_manager::utf16::SqliteWrapper::Create(database_path);
		if (!sqlite_wrapper_) {
			return false;
		}

		return true;
	}

	std::optional<int> ItemDao::GetItemId(const std::wstring& item_name, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT id FROM items WHERE name='%s' and parent_id=%d;", item_name.c_str(), parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			return std::nullopt;
		}

		auto row = result.value();
		if (row.empty()) {
			return std::nullopt;
		}

		return _wtoi(row[0][L"id"].c_str());
	}

	std::optional<common_utility::FileInfo> ItemDao::GetFileInfo(const std::wstring& file_name, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT name, size FROM items WHERE name='%s' and parent_id=%d;", file_name.c_str(), parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
			return std::nullopt;
		}

		auto row = result.value();
		if (row.empty()) {
			return std::nullopt;
		}

		common_utility::FileInfo info;
		info.name = row[0][L"name"];
		info.size = row[0][L"size"].empty() ? -1 : _wtoi(row[0][L"size"].c_str());

		return info;
	}

	std::optional<std::vector<common_utility::FileInfo>> ItemDao::GetFolderContainList(int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"SELECT name, size FROM items WHERE parent_id=%d;", parent_id);
		std::optional<std::vector<sqlite_manager::utf16::DataSet>> result = sqlite_wrapper_->ExecuteQuery(query);
		if (!result.has_value()) {
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

	std::optional<int> ItemDao::ChangeItemName(const common_utility::ChangeNameInfo& name_info, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"UPDATE items SET name='%s' WHERE name='%s' and parent_id=%d;", name_info.new_name.c_str(), name_info.old_name.c_str(), parent_id);
		int result = sqlite_wrapper_->ExecuteUpdate(query);
		if (result < 0) {
			return std::nullopt;
		}

		return result;
	}

	std::optional<int> ItemDao::DeleteItemInfo(const std::wstring& item_name, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"DELETE FROM items WHERE name='%s' and parent_id=%d;", item_name.c_str(), parent_id);
		int result = sqlite_wrapper_->ExecuteUpdate(query);
		if (result < 0) {
			return std::nullopt;
		}

		return result;
	}

	std::optional<int> ItemDao::InsertFileInfo(const common_utility::FileInfo& file_info, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"INSERT INTO items(size, name, parent_id) VALUES(%d, '%s', %d);", file_info.size, file_info.name.c_str(), parent_id);
		int result = sqlite_wrapper_->ExecuteUpdate(query);
		if (result < 0) {
			return std::nullopt;
		}

		return result;
	}

	std::optional<int> ItemDao::ModifyFileInfo(const common_utility::FileInfo& file_info, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"UPDATE items SET size=%d WHERE name='%s' and parent_id=%d;", file_info.size, file_info.name.c_str(), parent_id);
		int result = sqlite_wrapper_->ExecuteUpdate(query);
		if (result < 0) {
			return std::nullopt;
		}

		return result;
	}

	std::optional<int> ItemDao::InsertFolderInfo(const common_utility::FolderInfo& folder_info, int parent_id) {
		if (!sqlite_wrapper_) {
			return std::nullopt;
		}

		std::wstring query = common_utility::format_wstring(L"INSERT INTO items(name, parent_id) VALUES('%s', %d);", folder_info.name.c_str(), parent_id);
		int result = sqlite_wrapper_->ExecuteUpdate(query);
		if (result < 0) {
			return std::nullopt;
		}

		return result;
	}
}  // namespace monitor_client