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

	std::optional<common_utility::ItemInfo> LocalDb::GetItemInfo(const std::wstring& relative_path) const {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::GetItemInfo: item_dao_ is null" << std::endl;
			return std::nullopt;
		}

		std::optional<int> result = GetParentId(relative_path);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::GetItemInfo: GetParentId Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		std::wstring item_name;
		if (!common_utility::SplitPath(relative_path, nullptr, item_name)) {
			std::wcerr << L"LocalDb::GetItemInfo: SplitPath Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		int parent_id = result.value();
		return item_dao_->GetItemInfo(item_name, parent_id);
	}

	std::optional<std::vector<common_utility::ItemInfo>> LocalDb::GetFolderContainList(const std::wstring& relative_path /*= L""*/) const {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::GetFolderContainList: item_dao_ is null" << std::endl;
			return std::nullopt;
		}

		int item_id = 0;
		if (!relative_path.empty()) {
			std::optional<int> result = GetParentId(relative_path);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::GetFolderContainList: GetParentId Fail: " << relative_path << std::endl;
				return std::nullopt;
			}

			std::wstring item_name;
			if (!common_utility::SplitPath(relative_path, nullptr, item_name)) {
				std::wcerr << L"LocalDb::GetFolderContainList: SplitPath Fail: " << relative_path << std::endl;
				return std::nullopt;
			}

			int parent_id = result.value();
			std::optional<common_utility::ItemInfo> item_info = item_dao_->GetItemInfo(item_name, parent_id);
			if (!item_info.has_value()) {
				std::wcerr << L"LocalDb::GetFolderContainList: item_dao_->GetItemInfo Fail: " << relative_path << std::endl;
				return std::nullopt;
			}
			
			if (item_info.value().size >= 0) {
				std::wcerr << L"LocalDb::GetFolderContainList: file cannot have items: " << relative_path << std::endl;
				return std::nullopt;
			}

			result = item_dao_->GetItemId(item_name, parent_id);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::GetFolderContainList: GetItemId Fail: " << relative_path << std::endl;
				return std::nullopt;
			}

			item_id = result.value();
		}
		
		std::optional<std::vector<common_utility::ItemInfo>> get_contain_list = item_dao_->GetFolderContainList(item_id);
		if (!get_contain_list.has_value()) {
			std::wcerr << L"LocalDb::GetFolderContainList: item_dao_->GetFolderContainList Fail: " << item_id << std::endl;
			return std::nullopt;
		}

		return get_contain_list;
	}

	bool LocalDb::InsertItem(const common_utility::ItemInfo& item_info) {
		if (!item_dao_) {
			std::wcerr << L"LocalDb::InsertItem: item_dao_ is null" << std::endl;
			return false;
		}

		std::optional<int> result = GetParentId(item_info.name);
		if (!result.has_value()) {
			std::wcerr << L"LocalDb::InsertItem: GetParentId Fail: " << item_info.name << std::endl;
			return false;
		}

		int parent_id = result.value();

		std::wstring item_name;
		if (!common_utility::SplitPath(item_info.name, nullptr, item_name)) {
			std::wcerr << L"LocalDb::InsertItem: SplitPath Fail: " << item_info.name << std::endl;
			return false;
		}

		common_utility::ItemInfo insert_item_info = item_info;
		insert_item_info.name = item_name;

		std::optional<common_utility::ItemInfo> is_exist = item_dao_->GetItemInfo(item_name, parent_id);
		if (is_exist.has_value()) {
			result = item_dao_->ModifyItemInfo(insert_item_info, parent_id);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::InsertItem: ModifyItemInfo Fail: " << insert_item_info.name << L' ' << parent_id << std::endl;
				return false;
			}

			int insert_count = result.value();
			if (1 != insert_count) {
				std::wcerr << L"LocalDb::InsertItem: ModifyItemInfo Fail: " << insert_count << std::endl;
				return false;
			}
		}
		else {
			result = item_dao_->InsertItemInfo(insert_item_info, parent_id);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::InsertItem: InsertItemInfo Fail: " << insert_item_info.name << L' ' << parent_id << std::endl;
				return false;
			}

			int insert_count = result.value();
			if (1 != insert_count) {
				std::wcerr << L"LocalDb::InsertItem: InsertItemInfo Fail: " << insert_count << std::endl;
				return false;
			}
		}		

		return true;
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

		using RemoveInfo = std::pair<std::wstring, int>;
		std::vector<RemoveInfo> remove_list;
		std::queue<RemoveInfo> folder_list;

		int parent_id = result.value();
		remove_list.push_back({ item_name, parent_id });
		folder_list.push({ item_name, parent_id });

		while (folder_list.size() > 0) {
			RemoveInfo delete_info = folder_list.front();
			folder_list.pop();

			result = item_dao_->GetItemId(delete_info.first, delete_info.second);
			if (!result.has_value()) {
				std::wcerr << L"LocalDb::RemoveItem: GetItemId Fail: " << delete_info.first << L' ' << delete_info.second << std::endl;
				return false;
			}

			int item_id = result.value();
			std::optional<std::vector<common_utility::ItemInfo>> get_contain_list = item_dao_->GetFolderContainList(item_id);
			if (!get_contain_list.has_value()) {
				std::wcerr << L"LocalDb::RemoveItem: GetFolderContainList Fail: " << item_id << std::endl;
				return false;
			}

			const auto& contain_list = get_contain_list.value();
			for (const auto& contain : contain_list) {
				RemoveInfo contain_delete_info{ contain.name, item_id };
				remove_list.push_back(contain_delete_info);

				if (contain.size < 0) {
					folder_list.push(contain_delete_info);
				}
			}
		}

		int total_count = 0;
		for (const auto& delete_info : remove_list) {
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
}  // namespace monitor_client