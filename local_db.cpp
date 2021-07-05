#include "local_db.h"

#include <iostream>
#include <queue>
#include <vector>
#include <optional>
#include <string>
#include <utility>

#include "common_utility.h"

namespace monitor_client {
	LocalDb::LocalDb(std::unique_ptr<ItemDao>&& item_dao) : item_dao_(std::move(item_dao)) {

	}

	LocalDb::LocalDb(LocalDb&& rhs) : item_dao_(std::move(rhs.item_dao_)) {
		rhs.item_dao_ = nullptr;
	}

	LocalDb& LocalDb::operator=(LocalDb&& rhs) {
		std::swap(item_dao_, rhs.item_dao_);
		return *this;
	}

	std::optional<int> LocalDb::GetParentId(const std::wstring& relative_path) const {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::GetParentId: item_dao_ is null" << std::endl;
			return std::nullopt;
		}

		std::vector<std::wstring> split_parent_path;
		std::wstring item_name;

		if (!common_utility::SplitPath(relative_path, &split_parent_path, item_name)) {
			std::wcerr << L"LocalDb::GetParentId: SplitPath Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		if (split_parent_path.empty() && item_name.empty()) {
			std::wcerr << L"LocalDb::GetParentId: empty path" << std::endl;
			return std::nullopt;
		}

		if (split_parent_path.empty() && !item_name.empty()) {
			return 0;
		}

		int parent_id = 0;
		for (const auto& parent_path : split_parent_path) {
			std::optional<int> result = item_dao_->GetItemId(parent_path, parent_id);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::GetParentId: GetItemId Fail: " << parent_path << L' ' << parent_id << std::endl;
				return std::nullopt;
			}

			parent_id = result.value();
		}

		return parent_id;
	}

	std::optional<common_utility::FileInfo> LocalDb::GetFileInfo(const std::wstring& relative_path) const {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::GetFileInfo: item_dao_ is null" << std::endl;
			return std::nullopt;
		}

		std::optional<int> result = GetParentId(relative_path);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::GetFileInfo: GetParentId Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		std::wstring item_name;
		if (!common_utility::SplitPath(relative_path, nullptr, item_name)) {
			std::wcerr << L"LocalDb::GetFileInfo: SplitPath Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		int parent_id = result.value();
		return item_dao_->GetFileInfo(item_name, parent_id);
	}

	bool LocalDb::RenameItem(const common_utility::ChangeNameInfo& name_info) {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::RenameItem: item_dao_ is null" << std::endl;
			return false;
		}

		std::optional<int> result = GetParentId(name_info.old_name);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::RenameItem: GetParentId Fail: " << name_info.old_name << std::endl;
			return false;
		}

		std::wstring old_item_name;
		if (!common_utility::SplitPath(name_info.old_name, nullptr, old_item_name)) {
			std::wcerr << L"LocalDb::RenameItem: SplitPath Fail: " << name_info.old_name << std::endl;
			return false;
		}

		std::wstring new_item_name;
		if (!common_utility::SplitPath(name_info.new_name, nullptr, new_item_name)) {
			std::wcerr << L"LocalDb::RenameItem: SplitPath Fail: " << name_info.new_name << std::endl;
			return false;
		}

		common_utility::ChangeNameInfo info;
		info.old_name = old_item_name;
		info.new_name = new_item_name;

		int parent_id = result.value();
		result = item_dao_->ChangeItemName(info, parent_id);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::RenameItem: ChangeItemName Fail: " << info.old_name << L'?' << info.new_name << parent_id  << std::endl;
			return false;
		}

		int rename_count = result.value();
		if (1 != rename_count) {
			std::wcerr << L"LocalDb::RenameItem: ChangeItemName Fail: " << rename_count << std::endl;
			return false;
		}

		return true;
	}

	bool LocalDb::RemoveItem(const std::wstring& relative_path) {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::RemoveItem: item_dao_ is null" << std::endl;
			return false;
		}

		std::optional<int> result = GetParentId(relative_path);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::RemoveItem: GetParentId Fail: " << relative_path << std::endl;
			return false;
		}

		std::wstring item_name;
		if (!common_utility::SplitPath(relative_path, nullptr, item_name)) {
			std::wcerr << L"LocalDb::RemoveItem: SplitPath Fail: " << relative_path << std::endl;
			return false;
		}

		using DeleteInfo = std::pair<std::wstring, int>;
		std::vector<DeleteInfo> delete_list;
		std::queue<DeleteInfo> folder_list;

		int parent_id = result.value();
		delete_list.push_back(std::make_pair(item_name, parent_id));
		folder_list.push(std::make_pair(item_name, parent_id));

		while (folder_list.size() > 0) {
			DeleteInfo delete_info = folder_list.front();
			folder_list.pop();

			result = item_dao_->GetItemId(delete_info.first, delete_info.second);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::RemoveItem: GetItemId Fail: " << delete_info.first << L' ' << delete_info.second << std::endl;
				return false;
			}

			int item_id = result.value();
			std::optional<std::vector<common_utility::FileInfo>> get_contain_list = item_dao_->GetFolderContainList(item_id);
			if (!get_contain_list.has_value()) {
				std::wcerr << L"LocalDb::RemoveItem: GetFolderContainList Fail: " << item_id << std::endl;
				return false;
			}

			const auto& contain_list = get_contain_list.value();
			for (const auto& contain : contain_list) {
				DeleteInfo contain_delete_info = std::make_pair(contain.name, item_id);
				delete_list.push_back(contain_delete_info);

				if (contain.size < 0) {
					folder_list.push(contain_delete_info);
				}
			}
		}

		int total_count = 0;
		for (const auto& delete_info : delete_list) {
			result = item_dao_->DeleteItemInfo(delete_info.first, delete_info.second);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::RemoveItem: DeleteItemInfo Fail: " << delete_info.first << L' ' << delete_info.second << std::endl;
				return false;
			}

			total_count += result.value();
		}

		std::wclog << L"Delete item count: " << total_count << std::endl;
		return true;
	}

	bool LocalDb::AddFile(const common_utility::FileInfo& info) {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::AddFile: item_dao_ is null" << std::endl;
			return false;
		}

		std::optional<int> result = GetParentId(info.name);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::AddFile: GetParentId Fail: " << info.name << std::endl;
			return false;
		}

		int parent_id = result.value();

		std::wstring item_name;
		if (!common_utility::SplitPath(info.name, nullptr, item_name)) {
			std::wcerr << L"LocalDb::AddFile: SplitPath Fail: " << info.name << std::endl;
			return false;
		}

		std::optional<common_utility::FileInfo> is_exist = item_dao_->GetFileInfo(item_name, parent_id);
		if (is_exist.has_value()) {
			std::wclog << common_utility::format_wstring(L"%s is alreay exist", info.name);
			return true;
		}

		common_utility::FileInfo file_info = info;
		file_info.name = item_name;		

		result = item_dao_->InsertFileInfo(file_info, parent_id);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::AddFile: InsertFileInfo Fail: " << file_info.name << L' ' << parent_id << std::endl;
			return false;
		}

		int insert_count = result.value();
		if (1 != insert_count) {
			std::wcerr << L"LocalDb::AddFile: InsertFileInfo Fail: " << insert_count << std::endl;
			return false;
		}

		return true;
	}

	bool LocalDb::ModifyFile(const common_utility::FileInfo& info) {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::ModifyFile: item_dao_ is null" << std::endl;
			return false;
		}

		std::optional<int> result = GetParentId(info.name);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::ModifyFile: GetParentId Fail: " << info.name << std::endl;
			return false;
		}
		
		std::wstring item_name;
		if (!common_utility::SplitPath(info.name, nullptr, item_name)) {
			std::wcerr << L"LocalDb::ModifyFile: SplitPath Fail: " << info.name << std::endl;
			return false;
		}

		common_utility::FileInfo file_info = info;
		file_info.name = item_name;

		int parent_id = result.value();
		result = item_dao_->ModifyFileInfo(file_info, parent_id);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::ModifyFile: ModifyFileInfo Fail: " << file_info.name << L' ' << parent_id << std::endl;
			return false;
		}

		int modify_count = result.value();
		if (1 != modify_count) {
			std::wcerr << L"LocalDb::ModifyFile: ModifyFileInfo Fail: " << modify_count << std::endl;
			return false;
		}

		return true;
	}

	bool LocalDb::AddFolder(const common_utility::FolderInfo& info) {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::AddFolder: item_dao_ is null" << std::endl;
			return false;
		}

		std::optional<int> result = GetParentId(info.name);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::AddFolder: GetParentId Fail: " << info.name << std::endl;
			return false;
		}

		int parent_id = result.value();

		std::wstring item_name;
		if (!common_utility::SplitPath(info.name, nullptr, item_name)) {
			std::wcerr << L"LocalDb::AddFolder: SplitPath Fail: " << info.name << std::endl;
			return false;
		}

		std::optional<common_utility::FileInfo> is_exist = item_dao_->GetFileInfo(item_name, parent_id);
		if (is_exist.has_value()) {
			std::wclog << common_utility::format_wstring(L"%s is alreay exist", info.name);
			return true;
		}

		common_utility::FolderInfo folder_info = info;
		folder_info.name = item_name;

		result = item_dao_->InsertFolderInfo(folder_info, parent_id);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::AddFolder: InsertFolderInfo Fail: " << folder_info.name << L' ' << parent_id << std::endl;
			return false;
		}

		int insert_count = result.value();
		if (1 != insert_count) {
			std::wcerr << L"LocalDb::AddFile: InsertFileInfo Fail: " << insert_count << std::endl;
			return false;
		}

		return true;
	}
}  // namespace monitor_client