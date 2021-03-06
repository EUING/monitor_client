#ifndef MONITOR_CLIENT_DIFF_CHECK_H_
#define MONITOR_CLIENT_DIFF_CHECK_H_

#include <vector>
#include <string>
#include <memory>

#include "common_struct.h"
#include "common_utility.h"
#include "item_http.h"
#include "local_db.h"

namespace monitor_client {
namespace diff_check {
	struct DiffInfo {
		std::wstring prev_hash;
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
		std::vector<std::wstring> remove_list;
		std::vector<std::wstring> conflict_list;
	};

	LocalDiffList MakeLocalDiffList(const common_utility::ItemList& from_os, const common_utility::ItemList& from_db);
	ServerDiffList MakeServerDiffList(const common_utility::ItemList& from_server, const LocalDiffList& local_diff_list);
	ServerDiffList InitialDiffCheck(std::shared_ptr<LocalDb> local_db, std::shared_ptr<ItemHttp> item_http, const std::wstring& relative_path = L"");
}  // namespace diff_check
}  // namespace monitor_client
#endif