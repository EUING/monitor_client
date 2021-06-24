#include "file_manager.h"

#include <Windows.h>

#include <future>
#include <optional>
#include <sstream>
#include <iomanip>

#include "common_utility.h"
#include "notify_queue.h"

namespace monitor_client {
	FileManager::FileManager(NotifyQueue* notify_queue, FileHttp* file_http) : thread_future_{}, notify_queue_(notify_queue), file_http_(file_http) {
	}

	FileManager::~FileManager() {
		Stop();
	}

	bool FileManager::Run() {
		if (IsRunning())
			return false;

		thread_future_ = std::async(std::launch::async, [this]() {
			if (notify_queue_) {
				while (true) {				
					std::optional<common_utility::ChangeObjectInfo> pop = notify_queue_->Pop();
					if (!pop.has_value())
						return;

					auto change_info = pop.value();
					switch (change_info.action) {
					case FILE_ACTION_ADDED: {
						std::optional<common_utility::FileInfo> result = common_utility::GetFileInfo(change_info.full_path);
						if (result.has_value()) {
							if (file_http_) {
								file_http_->AddFile(result.value());
							}
						}
						break;
					}
					case FILE_ACTION_MODIFIED: {
						std::optional<common_utility::FileInfo> result = common_utility::GetFileInfo(change_info.full_path);
						if (result.has_value()) {
							if (file_http_) {
								file_http_->ModifyFile(result.value());
							}
						}
						break;
					}
					case FILE_ACTION_REMOVED: {
						std::optional<std::wstring> result = common_utility::GetFileName(change_info.full_path);
						if (result.has_value()) {
							if (file_http_) {
								file_http_->RemoveFile(result.value());
							}
						}
						break;
					}
					case FILE_ACTION_RENAMED_NEW_NAME: {
						std::optional<common_utility::ChangeNameInfo> result = common_utility::SplitChangeName(change_info.full_path);
						if (result.has_value()) {
							if (file_http_) {
								file_http_->RenameFile(result.value());
							}
						}
						break;
					}
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

	void FileManager::Stop() {
		if (IsRunning()) {
			notify_queue_->Break();
			thread_future_.wait();
		}
	}

	bool FileManager::IsRunning() {
		return thread_future_.valid() && std::future_status::timeout == thread_future_.wait_for(std::chrono::milliseconds(0));
	}
}  // namespace monitor_client