#include "diff_check.h"

#include <unordered_set>
#include <vector>
#include <string>

#include "common_utility.h"

namespace monitor_client {
namespace diff_check {
	LocalDiffList MakeLocalDiffList(const common_utility::ItemList& from_os, const common_utility::ItemList& from_db) {
		LocalDiffList local_diff_list;
		for (const auto& os : from_os) {
			auto db_iter = from_db.find(os);
			if (from_db.end() == db_iter) {
				local_diff_list.create_list.push_back(os);  // DB�� �ش� ������ ���� ���
			}
			else {
				common_utility::ItemInfo db_info = *db_iter;
				common_utility::ItemInfo os_info = os;
				if (db_info.hash != os_info.hash) {
					local_diff_list.modify_list.push_back({ db_info.hash, os_info });  // DB�� ������ �ؽ� ���� �ٸ� ���
				}
				else {
					local_diff_list.equal_list.push_back(os);
				}
			}
		}

		return local_diff_list;
	}

	ServerDiffList MakeServerDiffList(const common_utility::ItemList& from_server, const LocalDiffList& local_diff_list) {
		ServerDiffList server_diff_list;
		auto copy = from_server;  // ���� ����� ó���ϸ� �ش� ������ ����� ���� ����

		for (const auto& create : local_diff_list.create_list) {
			copy.erase(create);
			auto server_iter = from_server.find(create);
			if (from_server.end() != server_iter) {
				if (server_iter->hash != create.hash) {
					server_diff_list.conflict_list.push_back(create.name);  // �������ο��� ������ ���ϰ� �������� ������ ������ �ؽ� ���� �ٸ� ���
				}				
			}
			else {
				server_diff_list.upload_request_list.push_back(create);  // �������ο��� ������ ������ �������� �� ã�� ���
			}
		}

		for (const auto& modify : local_diff_list.modify_list) {
			copy.erase(modify.os_item);
			auto server_iter = from_server.find(modify.os_item);
			if (from_server.end() != server_iter) {
				if (server_iter->hash != modify.prev_hash) {
					server_diff_list.conflict_list.push_back(modify.os_item.name);  // �������ο��� ������ ������ DB �ؽ� ���� ���� �ؽ� ���� �ٸ� ���
				}
				else {
					server_diff_list.upload_request_list.push_back(modify.os_item);
				}
			}
			else {
				server_diff_list.upload_request_list.push_back(modify.os_item);  // �������� ������ ������ �������ο��� ������ ���
			}
		}

		for (const auto& equal : local_diff_list.equal_list) {
			copy.erase(equal);
			auto server_iter = from_server.find(equal);
			if (from_server.end() != server_iter) {
				if (server_iter->hash != equal.hash) {
					server_diff_list.download_request_list.push_back(equal.name);
				}
			}
			else {
				server_diff_list.remove_list.push_back(equal.name);  // �������� ������ ������ �������� ���� ���
			}
		}

		for (const auto& server_create_item : copy) {  // �������� ������ ����
			server_diff_list.download_request_list.push_back(server_create_item.name);
		}
		
		std::sort(server_diff_list.upload_request_list.begin(), server_diff_list.upload_request_list.end(), 
			[](const common_utility::ItemInfo& lhs, const common_utility::ItemInfo& rhs){
				return lhs.name < rhs.name;
		});

		std::sort(server_diff_list.download_request_list.begin(), server_diff_list.download_request_list.end());

		return server_diff_list;
	}

	ServerDiffList InitialDiffCheck(std::shared_ptr<LocalDb> local_db, std::shared_ptr<ItemHttp> item_http, const std::wstring& relative_path /*= L""*/) {
		common_utility::ItemList os_item_list;

		std::wstring window_root_path = relative_path.empty() ? L"." : relative_path;
		common_utility::GetSubFolderInfo(window_root_path, os_item_list);

		common_utility::ItemList db_item_list;
		common_utility::GetSubFolderInfo(*local_db, relative_path, db_item_list);

		common_utility::ItemList server_item_list;
		common_utility::GetSubFolderInfo(*item_http, relative_path, server_item_list);

		diff_check::LocalDiffList local_diff_list = diff_check::MakeLocalDiffList(os_item_list, db_item_list);
		diff_check::ServerDiffList server_diff_list = diff_check::MakeServerDiffList(server_item_list, local_diff_list);

		return server_diff_list;
	}
}  // namespace diff_check
}  // namespace monitor_client