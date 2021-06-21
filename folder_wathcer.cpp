#include "folder_watcher.h"

#include <Windows.h>

#include <future>
#include <iostream>
#include <queue>
#include <utility>
#include <string>
#include <optional>
#include <memory>

namespace my_rest_client {
	FolderWatcher::FolderWatcher(std::queue<std::pair<DWORD, std::wstring>>* change_info, const std::wstring& watch_folder /*= L""*/) 
		: thread_future_{}, stop_watching_event_(NULL), change_info_(change_info), watch_folder_(watch_folder) {
	}

	FolderWatcher::~FolderWatcher() {
		StopWatching();
		CloseEvent();
	}

	FolderWatcher::FolderWatcher(FolderWatcher&& rhs) {
		rhs.StopWatching();
		thread_future_ = std::move(rhs.thread_future_);
		watch_folder_ = std::move(rhs.watch_folder_);
		change_info_ = rhs.change_info_;
		rhs.change_info_ = nullptr;
		stop_watching_event_ = rhs.stop_watching_event_;
		rhs.stop_watching_event_ = NULL;
	}

	FolderWatcher& FolderWatcher::operator=(FolderWatcher&& rhs) {
		rhs.StopWatching();
		thread_future_ = std::move(rhs.thread_future_);
		watch_folder_ = std::move(rhs.watch_folder_);
		change_info_ = rhs.change_info_;
		rhs.change_info_ = nullptr;
		stop_watching_event_ = rhs.stop_watching_event_;
		rhs.stop_watching_event_ = NULL;

		return *this;
	}

	void FolderWatcher::CloseEvent() {
		if (NULL != stop_watching_event_) {
			CloseHandle(stop_watching_event_);
			stop_watching_event_ = NULL;
		}
	}

	void FolderWatcher::StopWatching() {
		if (NULL != stop_watching_event_) {
			SetEvent(stop_watching_event_);
			thread_future_.wait();
		}
	}

	bool FolderWatcher::StartWatching() {
		if (IsRunning()) {
			return false;
		}

		std::optional<std::shared_ptr<void>> result = InitWatching();
		if (!result.has_value()) {
			return false;
		}

		auto folder_handle = result.value();

		thread_future_ = std::async(std::launch::async, &FolderWatcher::WatchingDirectory, this, folder_handle);

		return true;
	}	

	std::optional<std::shared_ptr<void>> FolderWatcher::InitWatching() {
		DWORD attribute = GetFileAttributes(watch_folder_.c_str());
		if (INVALID_FILE_ATTRIBUTES == attribute) {
			std::wcerr << L"GetFileAttributes Fail!\n";
			return std::nullopt;
		}

		if (0 == (attribute & FILE_ATTRIBUTE_DIRECTORY)) {
			std::wcerr << L"No Folder!\n";
			return std::nullopt;  // 폴더가 아니면 감시하지 않음
		}

		HANDLE folder_handle = CreateFile(
			watch_folder_.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,  // 비동기로 처리하기 위한 플래그
			0);
		
		if (INVALID_HANDLE_VALUE == folder_handle) {
			std::wcerr << L"CreateFile Fail!\n";
			return std::nullopt;
		}

		auto invalid_deleter = [](HANDLE handle) {
			if (INVALID_HANDLE_VALUE != handle)
				CloseHandle(handle);

		};

		std::shared_ptr<void> folder_handle_ptr(folder_handle, invalid_deleter);

		stop_watching_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == stop_watching_event_) {
			std::wcerr << L"CreateEvent Fail!\n";
			return std::nullopt;
		}

		return folder_handle_ptr;
	}

	void FolderWatcher::WatchingDirectory(std::shared_ptr<void> folder_handle_ptr) {
		constexpr DWORD kBufferSize = 1024 * 1024;
		constexpr BOOL kWatchSubtree = FALSE;				
	    constexpr DWORD notify_filter =
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;		

		OVERLAPPED overlap{ 0, };
		overlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == overlap.hEvent) {
			std::wcerr << L"CreateEvent Fail!\n";
			return;
		}

		auto null_deleter = [](HANDLE handle) {
			if (NULL != handle)
				CloseHandle(handle);
		};

		std::unique_ptr<BYTE[]> buffer = std::make_unique<BYTE[]>(kBufferSize);
		std::unique_ptr<void, decltype(null_deleter)> overlap_handle_ptr(overlap.hEvent, null_deleter);
		
		HANDLE folder_handle = folder_handle_ptr.get();

		HANDLE handles[2] = { overlap_handle_ptr.get(), stop_watching_event_ };

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
				0);

			if (!result) {
				std::wcerr << L"ReadDirectoryChange Fail!\n";
				break;
			}

			DWORD signal = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
			if (WAIT_OBJECT_0 == signal) {
				if (!GetOverlappedResult(folder_handle, &overlap, &bytes_returned, FALSE)) {
					std::wcerr << L"GetOverlappedResult Fail!\n";
					break;
				}

				DWORD offset = 0;
				FILE_NOTIFY_INFORMATION* fni = nullptr;

				do {
					fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer.get()[offset]);

					std::wstring full_path;
					if (FILE_ACTION_RENAMED_OLD_NAME == fni->Action) {
						std::wstring old_name(fni->FileName, fni->FileNameLength / 2);
						std::wstring old_full_path = watch_folder_ + L"\\" + old_name;

						offset += fni->NextEntryOffset;
						fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer.get()[offset]);  // NEW_NAME을 알기 위해 offset을 증가

						std::wstring new_name(fni->FileName, fni->FileNameLength / 2);
						std::wstring new_full_path = watch_folder_ + L"\\" + new_name;
						full_path = old_full_path + L';' + new_full_path;
					}
					else {
						std::wstring name(fni->FileName, fni->FileNameLength / 2);
						full_path = watch_folder_ + L"\\" + name;
					}

					if (change_info_) {
						change_info_->push({ fni->Action, full_path });
					}

					offset += fni->NextEntryOffset;
				} while (fni->NextEntryOffset != 0);
			}
			else if ((WAIT_OBJECT_0 + 1) == signal) {
				std::wcout << L"User cancel!\n";
				break;
			}
			else {
				std::wcerr << L"WaitForMultipleObjects Fail!\n";
				break;
			}	
		}
		
		CloseEvent();
	}
}