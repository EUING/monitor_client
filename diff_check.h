#ifndef MONITOR_CLIENT_DIFF_CHECK_H_
#define MONITOR_CLIENT_DIFF_CHECK_H_

#include <unordered_set>
#include <vector>
#include <string>

#include "common_utility.h"
#include "item_http.h"
#include "local_db.h"

namespace monitor_client {
namespace common_utility {
	using ItemList = std::unordered_set<common_utility::ItemInfo>;
	void GetSubFolderInfo(const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const LocalDb& local_db, const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const ItemHttp& item_http, const std::wstring& folder_path, ItemList& item_list);
}  // namespace common_utility

namespace diff_check {
	struct DiffInfo {
		std::wstring other_hash;
		common_utility::ItemInfo os_item;
	};

	struct LocalDiffList {
		std::vector<common_utility::ItemInfo> create_list;
		std::vector<common_utility::ItemInfo> equal_list;
		std::vector<DiffInfo> modify_list;
	};

	struct ServerDiffList {
		std::vector<common_utility::ItemInfo> upload_request_list;
		std::vector<std::wstring> download_request_list;
		std::vector<std::wstring> delete_list;
		std::vector<std::wstring> conflict_list;
	};

	LocalDiffList MakeLocalDiffList(const common_utility::ItemList& from_os, const common_utility::ItemList& from_db);
	ServerDiffList MakeServerDiffList(const common_utility::ItemList& from_server, const LocalDiffList& local_diff_list);
}  // namespace diff_check
}  // namespace monitor_client
#endif