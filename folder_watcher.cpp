#include "folder_watcher.h"

#include <Windows.h>
#include <stdint.h>

#include <future>
#include <iostream>
#include <string>
#include <optional>
#include <memory>

#include "common_utility.h"
#include "notify_queue.h"

namespace monitor_client {
	FolderWatcher::FolderWatcher(std::shared_ptr<NotifyQueue> notify_queue, const std::wstring& watch_folder /*= L""*/)
		: thread_future_{}, stop_watching_event_(NULL), notify_queue_(notify_queue), watch_folder_(watch_folder) {
	}

	FolderWatcher::~FolderWatcher() {
		StopWatching();
		CloseEvent();
	}

	void FolderWatcher::CloseEvent() {
		if (NULL != stop_watching_event_) {
			CloseHandle(stop_watching_event_);
			stop_watching_event_ = NULL;
		}
	}

	bool FolderWatcher::IsRunning() {
		return thread_future_.valid() && std::future_status::timeout == thread_future_.wait_for(std::chrono::milliseconds(0));
	}

	void FolderWatcher::StopWatching() {
		if (NULL != stop_watching_event_) {
			SetEvent(stop_watching_event_);
			thread_future_.wait();
		}
	}

	bool FolderWatcher::StartWatching() {
		if (IsRunning()) {
			std::wcerr << L"FolderWatcher::StartWatching: FolderWatcher is running" << std::endl;
			return false;
		}

		std::shared_ptr<void> folder_handle_ptr;
		std::shared_ptr<void> overlap_event_ptr;

		if (!InitWatching(folder_handle_ptr, overlap_event_ptr)) {
			std::wcerr << L"FolderWatcher::StartWatching: InitWatching Fail" << std::endl;
			return false;
		}

		thread_future_ = std::async(std::launch::async, &FolderWatcher::WatchingDirectory, this, folder_handle_ptr, overlap_event_ptr);
		if (std::future_status::timeout != thread_future_.wait_for(std::chrono::milliseconds(100))) {
			// thread ready, deferred 여부 확인 및 thread가 준비 완료될 때까지 대기
			std::wcerr << L"FolderWatcher::StartWatching: thread wait_for Fail" << std::endl;
			return false;
		}

		return true;
	}	

	bool FolderWatcher::InitWatching(std::shared_ptr<void>& folder_handle_ptr, std::shared_ptr<void>& overlap_event_ptr) {
		std::optional<bool> is_dir = common_utility::IsDirectory(watch_folder_);
		if (!(is_dir.has_value() && is_dir.value())) {
			std::wcerr << L"FolderWatcher::InitWatching: watch_folder_ is not directory" << std::endl;
			return false;   // 폴더가 아니면 감시하지 않음
		}

		HANDLE folder_handle = CreateFile(
			watch_folder_.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,  // 비동기로 처리하기 위한 플래그
			NULL);
		
		if (INVALID_HANDLE_VALUE == folder_handle) {
			std::wcerr << L"FolderWatcher::InitWatching: CreateFile Fail: " << watch_folder_ << std::endl;
			return false;
		}

		if (!SetCurrentDirectory(watch_folder_.c_str())) {
			std::wcerr << L"FolderWatcher::InitWatching: SetCurrentDirectory Fail: " << watch_folder_ << std::endl;
			return false;
		}

		auto invalid_deleter = [](HANDLE handle) {
			if (INVALID_HANDLE_VALUE != handle)
				CloseHandle(handle);
		};

		folder_handle_ptr = std::shared_ptr<void>(folder_handle, invalid_deleter);

		auto null_deleter = [](HANDLE handle) {
			if (NULL != handle)
				CloseHandle(handle);
		};

		HANDLE overlap_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == overlap_event) {
			std::wcerr << L"FolderWatcher::InitWatching: CreateEvent Fail: " << "overlap_event" << std::endl;
			return false;
		}

		overlap_event_ptr = std::shared_ptr<void>(overlap_event, null_deleter);

		stop_watching_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == stop_watching_event_) {
			std::wcerr << L"FolderWatcher::InitWatching: CreateEvent Fail: " << "stop_watching_event_" << std::endl;
			return false;
		}

		HANDLE handles[2] = { overlap_event_ptr.get(), stop_watching_event_ };
		DWORD signal = WaitForMultipleObjects(2, handles, FALSE, 0);
		if (WAIT_TIMEOUT != signal) {  // handle valid check
			std::wcerr << L"FolderWatcher::InitWatching: WaitForMultipleObjects Fail: " << signal << std::endl;
			CloseEvent();
			return false;
		}

		return true;
	}

	void FolderWatcher::WatchingDirectory(std::shared_ptr<void> folder_handle_ptr, std::shared_ptr<void> overlap_event_ptr) {
		HANDLE folder_handle = folder_handle_ptr.get();
		HANDLE overlap_event = overlap_event_ptr.get();

		constexpr DWORD kBufferSize = 1024 * 1024;
		constexpr BOOL kWatchSubtree = TRUE;
		constexpr DWORD notify_filter =
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

		std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(kBufferSize);
		HANDLE handles[2] = { overlap_event, stop_watching_event_ };

		OVERLAPPED overlap{ 0, };
		overlap.hEvent = overlap_event;

		while (true) {
			DWORD bytes_returned = 0;
			BOOL result = ReadDirectoryChangesW(
				folder_handle,
				buffer.get(),
				kBufferSize,
				kWatchSubtree,
				notify_filter,
				&bytes_returned,
				&overlap,
				NULL);

			if (!result) {
				std::wcerr << L"FolderWatcher::WatchingDirectory: ReadDirectoryChange Fail" << std::endl;
				break;
			}

			DWORD signal = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if (WAIT_OBJECT_0 == signal) {
				if (!GetOverlappedResult(folder_handle, &overlap, &bytes_returned, FALSE)) {
					std::wcerr << L"FolderWatcher::WatchingDirectory: GetOverlappedResult Fail" << std::endl;
					break;
				}

				FILE_NOTIFY_INFORMATION* fni = nullptr;
				bool is_first = true;

				for (DWORD offset = 0; is_first || fni->NextEntryOffset != 0; offset += fni->NextEntryOffset) {
					is_first = false;
					fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer.get()[offset]);

					std::wstring result_name;
					if (FILE_ACTION_ADDED == fni->Action) {
						std::wstring name(fni->FileName, fni->FileNameLength / 2);
						PushItem(name);
						continue;
					}
					else if (FILE_ACTION_RENAMED_OLD_NAME == fni->Action) {
						std::wstring old_name(fni->FileName, fni->FileNameLength / 2);

						offset += fni->NextEntryOffset;
						fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer.get()[offset]);  // NEW_NAME을 알기 위해 offset을 증가

						std::wstring new_name(fni->FileName, fni->FileNameLength / 2);
						result_name = old_name + L'?' + new_name;  // 기존 파일명과 새로운 파일명을 '?'으로 구분
					}
					else {
						std::wstring name(fni->FileName, fni->FileNameLength / 2);
						result_name = name;
					}

					std::wclog << fni->Action << L" " << result_name << std::endl;
					if (notify_queue_) {
						notify_queue_->Push({ fni->Action, result_name });
					}
				}
			}
			else if ((WAIT_OBJECT_0 + 1) == signal) {
				break;
			}
			else {
				std::wcerr << L"FolderWatcher::WatchingDirectory: WaitForMultipleObjects Fail" << std::endl;
				break;
			}
		}
		
		CloseEvent();
	}

	void FolderWatcher::PushItem(const std::wstring& relative_path) {
		std::wclog << FILE_ACTION_ADDED << L" " << relative_path << std::endl;
		notify_queue_->Push({ FILE_ACTION_ADDED, relative_path });

		std::optional<bool> is_dir = common_utility::IsDirectory(relative_path);
		if (!is_dir.has_value()) {
			std::wcerr << L"FolderWatcher::PushItem: IsDirectory Fail: "  << relative_path << std::endl;
			return;
		}		

		if (!is_dir.value()) {
			return;
		}

		WIN32_FIND_DATA find_data;
		std::wstring fname = relative_path + L"\\*.*";

		HANDLE handle = FindFirstFile(fname.c_str(), &find_data);
		if (INVALID_HANDLE_VALUE != handle) {
			do {
				std::wstring file_name(find_data.cFileName);
				if (L"." == file_name || L".." == file_name) {
					continue;
				}

				std::wstring relative_path_name = relative_path + L"\\" + file_name;
				PushItem(relative_path_name);

			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}
}  // namespace monitor_client