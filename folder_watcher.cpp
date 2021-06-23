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

namespace my_rest_client {
	FolderWatcher::FolderWatcher(NotifyQueue* notify_queue, const std::wstring& watch_folder /*= L""*/)
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

		std::shared_ptr<void> folder_handle_ptr;
		std::shared_ptr<void> overlap_event_ptr;

		if (!InitWatching(folder_handle_ptr, overlap_event_ptr)) {
			return false;
		}

		thread_future_ = std::async(std::launch::async, &FolderWatcher::WatchingDirectory, this, folder_handle_ptr, overlap_event_ptr);
		if (std::future_status::timeout != thread_future_.wait_for(std::chrono::milliseconds(100))) {
			// thread ready, deferred ���� Ȯ�� �� thread�� �غ� �Ϸ�� ������ ���
			return false;
		}

		return true;
	}	

	bool FolderWatcher::InitWatching(std::shared_ptr<void>& folder_handle_ptr, std::shared_ptr<void>& overlap_event_ptr) {
		DWORD attribute = GetFileAttributes(watch_folder_.c_str());
		if (INVALID_FILE_ATTRIBUTES == attribute) {
			std::wcerr << L"GetFileAttributes Fail!\n";
			return false;
		}

		if (0 == (attribute & FILE_ATTRIBUTE_DIRECTORY)) {
			std::wcerr << L"No Folder!\n";
			return false;  // ������ �ƴϸ� �������� ����
		}

		HANDLE folder_handle = CreateFile(
			watch_folder_.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,  // �񵿱�� ó���ϱ� ���� �÷���
			0);
		
		if (INVALID_HANDLE_VALUE == folder_handle) {
			std::wcerr << L"CreateFile Fail!\n";
			return false;
		}

		auto invalid_deleter = [](HANDLE handle) {
			if (INVALID_HANDLE_VALUE != handle)
				CloseHandle(handle);
		};

		folder_handle_ptr = std::shared_ptr<void>(folder_handle, invalid_deleter);

		stop_watching_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == stop_watching_event_) {
			std::wcerr << L"CreateEvent Fail!\n";
			return false;
		}

		auto null_deleter = [](HANDLE handle) {
			if (NULL != handle)
				CloseHandle(handle);
		};

		HANDLE overlap_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == overlap_event) {
			std::wcerr << L"CreateEvent Fail!\n";
			return false;
		}

		overlap_event_ptr = std::shared_ptr<void>(overlap_event, null_deleter);

		HANDLE ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == ready_event) {
			std::wcerr << L"CreateEvent Fail!\n";
			return false;
		}

		HANDLE handles[2] = { overlap_event_ptr.get(), stop_watching_event_ };
		DWORD signal = WaitForMultipleObjects(2, handles, FALSE, 0);
		if (WAIT_TIMEOUT != signal) {  // handle valid check
			std::wcerr << L"WaitForMultipleObjects Fail!\n";
			return false;
		}

		return true;
	}

	void FolderWatcher::WatchingDirectory(std::shared_ptr<void> folder_handle_ptr, std::shared_ptr<void> overlap_event_ptr) {
		HANDLE folder_handle = folder_handle_ptr.get();
		HANDLE overlap_event = overlap_event_ptr.get();

		constexpr DWORD kBufferSize = 1024 * 1024;
		constexpr BOOL kWatchSubtree = FALSE;
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
						fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&buffer.get()[offset]);  // NEW_NAME�� �˱� ���� offset�� ����

						std::wstring new_name(fni->FileName, fni->FileNameLength / 2);
						std::wstring new_full_path = watch_folder_ + L"\\" + new_name;
						full_path = old_full_path + L':' + new_full_path;  // ���� ���ϸ��� ���ο� ���ϸ��� ':'���� ����
					}
					else {
						std::wstring name(fni->FileName, fni->FileNameLength / 2);
						full_path = watch_folder_ + L"\\" + name;
					}

					std::wclog << fni->Action << L" " << full_path;
					if (notify_queue_) {
						notify_queue_->Push({ fni->Action, full_path });
					}

					offset += fni->NextEntryOffset;
				} while (fni->NextEntryOffset != 0);
			}
			else if ((WAIT_OBJECT_0 + 1) == signal) {
				break;
			}
			else {
				std::wcerr << L"WaitForMultipleObjects Fail!\n";
				break;
			}
		}
		
		CloseEvent();
	}
}  // namespace my_rest_client