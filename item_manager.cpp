#include "item_manager.h"

#include <Windows.h>

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
					std::optional<common_utility::ChangeItemInfo> pop = notify_queue_->Pop();
					if (!pop.has_value())
						return;

					auto change_info = pop.value();
					ManagementItem(change_info);
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

	void ItemManager::ManagementItem(const common_utility::ChangeItemInfo& info) {
		switch (info.action) {
		case FILE_ACTION_ADDED: {
			std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(info.relative_path);
			if (1 == result.index()) {
				common_utility::FileInfo file_info = std::get<common_utility::FileInfo>(result);
				std::replace(file_info.name.begin(), file_info.name.end(), L'\\', L'/');  // Window path to Posix path
				item_http_.AddFile(file_info);
			}
			else if (2 == result.index()) {
				common_utility::FolderInfo folder_info = std::get<common_utility::FolderInfo>(result);
				std::replace(folder_info.name.begin(), folder_info.name.end(), L'\\', L'/');  // Window path to Posix path
				item_http_.AddFolder(folder_info);
				ManagementSubFolder(info.relative_path);
			}

			break;
		}
		case FILE_ACTION_MODIFIED: {
			std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(info.relative_path);
			if (1 == result.index()) {
				common_utility::FileInfo file_info = std::get<common_utility::FileInfo>(result);
				std::replace(file_info.name.begin(), file_info.name.end(), L'\\', L'/');  // Window path to Posix path
				item_http_.ModifyFile(file_info);
			}  // 폴더 MODIFIED는 SKIP
			break;
		}
		case FILE_ACTION_REMOVED: {
			std::wstring item_path = info.relative_path;
			std::replace(item_path.begin(), item_path.end(), L'\\', L'/');  // Window path to Posix path
			item_http_.RemoveItem(item_path);			
			break;
		}
		case FILE_ACTION_RENAMED_NEW_NAME: {
			std::optional<common_utility::ChangeNameInfo> result = common_utility::SplitChangeName(info.relative_path);
			if (result.has_value()) {
				common_utility::ChangeNameInfo change_name_info;
				std::replace(change_name_info.old_name.begin(), change_name_info.old_name.end(), L'\\', L'/');  // Window path to Posix path
				std::replace(change_name_info.new_name.begin(), change_name_info.new_name.end(), L'\\', L'/');  // Window path to Posix path
				item_http_.RenameItem(change_name_info);
			}
			break;
		}
		}
	}

	void ItemManager::ManagementSubFolder(const std::wstring& relative_path) {
		WIN32_FIND_DATA find_data;
		std::wstring fname = relative_path + L"\\*.*";

		HANDLE handle = FindFirstFile(fname.c_str(), &find_data);
		if (INVALID_HANDLE_VALUE != handle) {
			do {
				std::wstring file_name(find_data.cFileName);
				if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (L"." != file_name && L".." != file_name) {
						std::wstring relative_path_name = relative_path + L"\\" + file_name;
						std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(relative_path_name);

						if (result.index() == 2) {
							common_utility::FolderInfo info = std::get<common_utility::FolderInfo>(result);
							std::replace(info.name.begin(), info.name.end(), L'\\', L'/');  // Window path to Posix path
							item_http_.AddFolder(info);
						}						

						ManagementSubFolder(relative_path_name);
					}
				}
				else {
					std::wstring relative_path_name = relative_path + L"\\" + file_name;
					std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(relative_path_name);
					
					if (result.index() == 1) {
						common_utility::FileInfo info = std::get<common_utility::FileInfo>(result);
						std::replace(info.name.begin(), info.name.end(), L'\\', L'/');  // Window path to Posix path
						item_http_.AddFile(info);
					}					
				}
			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}
}  // namespace monitor_client