#include "item_manager.h"

#include <Windows.h>

#include <variant>
#include <future>
#include <optional>

#include "common_utility.h"
#include "notify_queue.h"
#include "item_http.h"

namespace monitor_client {
	ItemManager::ItemManager(NotifyQueue* notify_queue, ItemHttp* item_http) : thread_future_{}, notify_queue_(notify_queue), item_http_(item_http) {
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
				if (item_http_) {
					item_http_->AddFile(std::get<common_utility::FileInfo>(result));
				}
			}
			else if (2 == result.index()) {
				if (item_http_) {
					item_http_->AddFolder(std::get<common_utility::FolderInfo>(result));
				}
				ManagementSubFolder(info.relative_path);
			}

			break;
		}
		case FILE_ACTION_MODIFIED: {
			std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(info.relative_path);
			if (1 == result.index()) {
				if (item_http_) {
					item_http_->ModifyFile(std::get<common_utility::FileInfo>(result));
				}
				// 폴더 MODIFIED는 SKIP
			}
			break;
		}
		case FILE_ACTION_REMOVED: {
			if (item_http_) {
				item_http_->RemoveItem(info.relative_path);
			}			
			
			break;
		}
		case FILE_ACTION_RENAMED_NEW_NAME: {
			std::optional<common_utility::ChangeNameInfo> result = common_utility::SplitChangeName(info.relative_path);
			if (result.has_value()) {
				if (item_http_) {
					item_http_->RenameItem(result.value());
				}
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

						if (item_http_) {
							item_http_->AddFolder(std::get<common_utility::FolderInfo>(result));
						}

						ManagementSubFolder(relative_path_name);
					}
				}
				else {
					std::wstring relative_path_name = relative_path + L"\\" + file_name;
					std::variant<std::monostate, common_utility::FileInfo, common_utility::FolderInfo> result = common_utility::GetItemInfo(relative_path_name);
					
					if (item_http_) {
						item_http_->AddFile(std::get<common_utility::FileInfo>(result));
					}
				}
			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}
}  // namespace monitor_client