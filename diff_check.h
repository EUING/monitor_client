#ifndef MONITOR_CLIENT_DIFF_CHECK_H_
#define MONITOR_CLIENT_DIFF_CHECK_H_

#include <unordered_map>
#include <vector>
#include <string>

#include "common_utility.h"
#include "item_http.h"
#include "local_db.h"

namespace monitor_client {
namespace common_utility {
	using ItemList = std::unordered_map<std::wstring, common_utility::ItemInfo>;
	void GetSubFolderInfo(const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const LocalDb& local_db, const std::wstring& folder_path, ItemList& item_list);
	//void GetSubFolderInfo(const ItemHttp& item_http, const std::wstring& folder_path, ItemList& item_list);
}  // namespace common_utility

namespace diff_check {
	struct DiffInfo {
		common_utility::ItemInfo db_item;
		common_utility::ItemInfo os_item;
	};

	struct DiffList {
		std::vector<common_utility::ItemInfo> create_list;
		std::vector<common_utility::ItemInfo> equal_list;
		std::vector<DiffInfo> modify_list;
	};

	DiffList MakeDiffList(const common_utility::ItemList& from_os, const common_utility::ItemList& from_db);
}  // namespace diff_check
}  // namespace monitor_client

#endif