#ifndef MONITOR_CLIENT_DIFF_CHECK_H_
#define MONITOR_CLIENT_DIFF_CHECK_H_

#include <unordered_set>
#include <vector>
#include <string>
#include <set>

#include "common_utility.h"
#include "item_http.h"
#include "local_db.h"

namespace monitor_client {
namespace diff_check {
	struct DiffInfo {
		common_utility::ItemInfo other_item;
		common_utility::ItemInfo os_item;

		friend bool operator==(const DiffInfo& lhs, const DiffInfo& rhs);
	};
}  // namespace diff_check
}  // namespace monitor_client

namespace std {
	template<>
	struct hash<monitor_client::diff_check::DiffInfo> {
		std::size_t operator()(const monitor_client::diff_check::DiffInfo& info) const noexcept {
			hash<wstring> hash_func;
			return hash_func(info.os_item.name);
		}
	};
}  // namespace std

namespace monitor_client {
namespace common_utility {
	using ItemList = std::unordered_set<common_utility::ItemInfo>;
	void GetSubFolderInfo(const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const LocalDb& local_db, const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const ItemHttp& item_http, const std::wstring& folder_path, ItemList& item_list);
}  // namespace common_utility

namespace diff_check {
	struct LocalDiffList {
		std::set<common_utility::ItemInfo> create_list;  // 생성된 목록은 오름차순으로 PUT 해야 함
		common_utility::ItemList equal_list;
		std::unordered_set<DiffInfo> modify_list;
	};

	struct ServerDiffList {
		std::vector<common_utility::ItemInfo> upload_request_list;
		std::vector<std::wstring> download_request_list;
		std::vector<std::wstring> delete_list;
		std::vector<DiffInfo> conflict_list;
	};

	LocalDiffList MakeLocalDiffList(const common_utility::ItemList& from_os, const common_utility::ItemList& from_db);
	ServerDiffList MakeServerDiffList(const common_utility::ItemList& from_server, const LocalDiffList& local_diff_list);
}  // namespace diff_check
}  // namespace monitor_client
#endif