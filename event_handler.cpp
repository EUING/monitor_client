#include "event_handler.h"

#include <Windows.h>
#include <stdint.h>

#include <optional>
#include <iostream>
#include <string>

#include "common_utility.h"
#include "notify_queue.h"

namespace monitor_client {
	EventHandler::EventHandler(const std::shared_ptr<NotifyQueue>& notify_queue) : notify_queue_(notify_queue) {
	}

	void EventHandler::PushEvent(std::shared_ptr<const uint8_t[]> buffer) {
		const FILE_NOTIFY_INFORMATION* fni = nullptr;
		bool is_first = true;

		for (DWORD offset = 0; is_first || fni->NextEntryOffset != 0; offset += fni->NextEntryOffset) {
			is_first = false;
			fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&buffer[offset]);

			if (FILE_ACTION_ADDED == fni->Action) {
				std::wstring name(fni->FileName, fni->FileNameLength / 2);
				PushAddEvent(name);
				continue;
			}
			else {
				std::wstring result_name;
				if (FILE_ACTION_RENAMED_OLD_NAME == fni->Action) {
					std::wstring old_name(fni->FileName, fni->FileNameLength / 2);

					offset += fni->NextEntryOffset;
					fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&buffer[offset]);  // NEW_NAME을 알기 위해 offset을 증가

					std::wstring new_name(fni->FileName, fni->FileNameLength / 2);
					result_name = old_name + L'?' + new_name;  // 기존 파일명과 새로운 파일명을 '?'으로 구분
				}
				else if (FILE_ACTION_MODIFIED == fni->Action) {
					std::wstring name(fni->FileName, fni->FileNameLength / 2);
					std::optional<bool> is_dir = common_utility::IsDirectory(name);
					if (!is_dir.has_value()) {
						std::wcerr << L"EventHandler::PushEvent: IsDirectory Fail: " << name << std::endl;
						continue;
					}

					if (is_dir.value()) {
						continue;  // 폴더 수정 이벤트는 무시
					}

					result_name = name;
				}
				else {
					std::wstring name(fni->FileName, fni->FileNameLength / 2);
					result_name = name;
				}

				std::wclog << fni->Action << L' ' << result_name << std::endl;
				if (notify_queue_) {
					notify_queue_->Push({ fni->Action, result_name });
				}
			}
		}
	}

	void EventHandler::PushAddEvent(const std::wstring& relative_path) {
		std::wclog << FILE_ACTION_ADDED << L' ' << relative_path << std::endl;
		if (notify_queue_) {
			notify_queue_->Push({ FILE_ACTION_ADDED, relative_path });
		}

		std::optional<bool> is_dir = common_utility::IsDirectory(relative_path);
		if (!is_dir.has_value()) {
			std::wcerr << L"EventHandler::PushAddEvent: IsDirectory Fail: " << relative_path << std::endl;
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
				PushAddEvent(relative_path_name);

			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}
}  // namespace monitor_client