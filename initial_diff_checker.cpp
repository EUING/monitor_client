#include "initial_diff_checker.h"

#include <memory>

#include "common_struct.h"
#include "common_utility.h"
#include "diff_check.h"

namespace monitor_client {
	InitialDiffChecker::InitialDiffChecker(const std::shared_ptr<LocalDb>& local_db, const std::shared_ptr<ItemHttp>& item_http) : local_db_(local_db), item_http_(item_http) {
	}

	diff_check::ServerDiffList InitialDiffChecker::FindDifference(const std::wstring& relative_path /*= L""*/) const {
		common_utility::ItemList os_item_list;
		
		std::wstring window_root_path = relative_path.empty() ? L"." : relative_path;
		common_utility::GetSubFolderInfo(window_root_path, os_item_list);

		common_utility::ItemList db_item_list;
		common_utility::GetSubFolderInfo(*local_db_, relative_path, db_item_list);

		common_utility::ItemList server_item_list;
		common_utility::GetSubFolderInfo(*item_http_, relative_path, server_item_list);
		
		diff_check::LocalDiffList local_diff_list =  diff_check::MakeLocalDiffList(os_item_list, db_item_list);
		diff_check::ServerDiffList server_diff_list = diff_check::MakeServerDiffList(server_item_list, local_diff_list);

		return server_diff_list;
	}
}  // monitor_client
