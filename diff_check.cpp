#include "diff_check.h"

#include <unordered_map>
#include <vector>
#include <string>

#include "common_utility.h"

namespace monitor_client {
namespace common_utility {
	void GetSubFolderInfo(const std::wstring& folder_path, ItemList& item_list) {
		WIN32_FIND_DATA find_data;
		std::wstring fname = folder_path + L"/*.*";

		HANDLE handle = FindFirstFile(fname.c_str(), &find_data);
		if (INVALID_HANDLE_VALUE != handle) {
			do {
				std::wstring file_name(find_data.cFileName);
				if (L"." == file_name || L".." == file_name) {
					continue;
				}

				std::wstring relative_path_name = (folder_path == L".") || (folder_path == L"..") ? file_name : folder_path + L'/' + file_name;
				std::optional<ItemInfo> item_info = GetItemInfo(relative_path_name);
				if (!item_info.has_value()) {
					std::wcerr << L"common_utility::GetSubFolderInfo: GetItemInfo Fail: " << relative_path_name << std::endl;
					return;
				}

				item_list.insert(std::make_pair(relative_path_name, item_info.value()));

				GetSubFolderInfo(relative_path_name, item_list);
			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}

	void GetSubFolderInfo(const LocalDb& local_db, const std::wstring& folder_path, ItemList& item_list) {
		std::optional<std::vector<common_utility::ItemInfo>> sub_item = local_db.GetFolderContainList(folder_path);
		if (!sub_item.has_value()) {
			std::wcerr << L"FolderSynchronizer::Sycnchronize: local_db_.GetFolderContainList Fail: " << folder_path << std::endl;
			return;
		}

		auto sub_item_list = sub_item.value();
		for (const auto& item : sub_item_list) {
			std::wstring relative_path = folder_path.empty() ? item.name : folder_path + L'/' + item.name;
			item_list.insert(std::make_pair(relative_path, item));

			if (item.size < 0) {
				GetSubFolderInfo(relative_path, item_list);
			}
		}
	}
}  // namespace common_utility

namespace diff_check {
	DiffList MakeDiffList(const common_utility::ItemList& from_os, const common_utility::ItemList& from_db) {
		DiffList diff_list;
		for (const auto& os : from_os) {
			if (from_db.end() == from_db.find(os.first)) {
				diff_list.create_list.push_back(os.second);
			}
			else {
				common_utility::ItemInfo db_info = from_db.at(os.first);
				common_utility::ItemInfo os_info = os.second;
				if (db_info.hash != os_info.hash) {
					diff_list.modify_list.push_back({ db_info, os.second });
				}
				else {
					diff_list.equal_list.push_back(os.second);
				}
			}
		}

		return diff_list;
	}
}  // namespace diff_check
}  // namespace monitor_client