#include "item_manager.h"

#include <Windows.h>

#include <iostream>
#include <chrono>
#include <variant>
#include <future>
#include <optional>
#include <algorithm>

#include "common_utility.h"
#include "notify_queue.h"
#include "item_http.h"

namespace monitor_client {
	ItemManager::ItemManager(std::shared_ptr<NotifyQueue> notify_queue, const common_utility::NetworkInfo& network_info, std::unique_ptr<ItemDao>&& item_dao) :
		thread_future_{}, 
		notify_queue_(notify_queue), 
		item_http_(network_info),
		local_db_(std::move(item_dao)) {

	}

	ItemManager::~ItemManager() {
		Stop();
	}

	bool ItemManager::Run() {
		if (IsRunning())
			return false;

		thread_future_ = std::async(std::launch::async, [this]() {
			if (notify_queue_) {
				while (true) {				
					std::optional<common_utility::ChangeItemInfo> front = notify_queue_->Front();
					if (!front.has_value())
						return;

					auto change_info = front.value();
					if (ManagementItem(change_info)) {
						notify_queue_->Pop();
					}
					else {
						std::wcerr << L"ItemManager::Run: ManagementItem Fail: " << change_info.relative_path << std::endl;
						std::this_thread::sleep_for(std::chrono::seconds(3));
					}
				}
			}
		});

		if (std::future_status::timeout != thread_future_.wait_for(std::chrono::milliseconds(100))) {
			// thread가 준비 완료될 때까지 대기
			return false;
		}

		return true;
	}

	void ItemManager::Stop() {
		if (IsRunning()) {
			notify_queue_->Break();
			thread_future_.wait();
		}
	}

	bool ItemManager::IsRunning() {
		return thread_future_.valid() && std::future_status::timeout == thread_future_.wait_for(std::chrono::milliseconds(0));
	}

	bool ItemManager::ManagementItem(const common_utility::ChangeItemInfo& info) {
		switch (info.action) {
		case FILE_ACTION_ADDED: {
			std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(info.relative_path);
			if (1 == result.index()) {
				common_utility::FileInfo file_info = std::get<common_utility::FileInfo>(result);
				std::replace(file_info.name.begin(), file_info.name.end(), L'\\', L'/');  // Window path to Posix path
				if (!item_http_.AddFile(file_info)) {
					std::wcerr << L"ItemManager::ManagementItem: item_http_.AddFile Fail: " << file_info.name << std::endl;
					return false;
				}
				
				if (!local_db_.AddFile(file_info)) {
					return false;
				}
			}
			else if (2 == result.index()) {
				common_utility::FolderInfo folder_info = std::get<common_utility::FolderInfo>(result);
				std::replace(folder_info.name.begin(), folder_info.name.end(), L'\\', L'/');  // Window path to Posix path
				if (!item_http_.AddFolder(folder_info)) {
					std::wcerr << L"ItemManager::ManagementItem: item_http_.AddFolder Fail: " << folder_info.name << std::endl;
					return false;
				}

				if (!local_db_.AddFolder(folder_info)) {
					std::wcerr << L"ItemManager::ManagementItem: local_db_.AddFolder Fail: " << folder_info.name << std::endl;
					return false;
				}
			}

			break;
		}
		case FILE_ACTION_MODIFIED: {
			std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(info.relative_path);
			if (1 == result.index()) {
				common_utility::FileInfo file_info = std::get<common_utility::FileInfo>(result);
				std::replace(file_info.name.begin(), file_info.name.end(), L'\\', L'/');  // Window path to Posix path
				if (!item_http_.ModifyFile(file_info)) {
					std::wcerr << L"ItemManager::ManagementItem: item_http_.ModifyFile Fail: " << file_info.name << std::endl;
					return false;
				}

				if (!local_db_.ModifyFile(file_info)) {
					std::wcerr << L"ItemManager::ManagementItem: local_db_.ModifyFile Fail: " << file_info.name << std::endl;
					return false;
				}
			}  // 폴더 MODIFIED는 SKIP
			break;
		}
		case FILE_ACTION_REMOVED: {
			std::wstring item_path = info.relative_path;
			std::replace(item_path.begin(), item_path.end(), L'\\', L'/');  // Window path to Posix path
			if (!item_http_.RemoveItem(item_path)) {
				std::wcerr << L"ItemManager::ManagementItem: item_http_.RemoveItem Fail: " << item_path << std::endl;
				return false;
			}

			if (!local_db_.RemoveItem(item_path)) {
				std::wcerr << L"ItemManager::ManagementItem: local_db_.RemoveItem Fail: " << item_path << std::endl;
				return false;
			}
			break;
		}
		case FILE_ACTION_RENAMED_NEW_NAME: {
			std::optional<common_utility::ChangeNameInfo> result = common_utility::SplitChangeName(info.relative_path);
			if (result.has_value()) {
				common_utility::ChangeNameInfo change_name_info;
				std::replace(change_name_info.old_name.begin(), change_name_info.old_name.end(), L'\\', L'/');  // Window path to Posix path
				std::replace(change_name_info.new_name.begin(), change_name_info.new_name.end(), L'\\', L'/');  // Window path to Posix path
				if (!item_http_.RenameItem(change_name_info)) {
					std::wcerr << L"ItemManager::ManagementItem: item_http_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
					return false;
				}

				if (!local_db_.RenameItem(change_name_info)) {
					std::wcerr << L"ItemManager::ManagementItem: local_db_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
					return false;
				}
			}
			break;
		}
		}

		return true;
	}
}  // namespace monitor_client