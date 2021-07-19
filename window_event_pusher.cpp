#include "window_event_pusher.h"

#include <Windows.h>
#include <stdint.h>

#include <string>
#include <algorithm>
#include <optional>
#include <memory>

#include "common_struct.h"
#include "common_utility.h"
#include "base_event_filter.h"
#include "event_queue.h"

namespace monitor_client {
	void WindowEventPusher::PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const {
		const FILE_NOTIFY_INFORMATION* fni = nullptr;
		bool is_first = true;

		for (DWORD offset = 0; is_first || fni->NextEntryOffset != 0; offset += fni->NextEntryOffset) {
			is_first = false;
			fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&buffer_[offset]);

			std::wstring name(fni->FileName, fni->FileNameLength / 2);
			if (common_utility::HasIgnore(name)) {
				return;
			}

			if (FILE_ACTION_ADDED == fni->Action) {
				PushAddEvent(event_filter, event_queue, name);
				continue;
			}
			else {
				if (FILE_ACTION_RENAMED_OLD_NAME == fni->Action) {
					offset += fni->NextEntryOffset;
					fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&buffer_[offset]);  // NEW_NAME을 알기 위해 offset을 증가

					std::wstring new_name(fni->FileName, fni->FileNameLength / 2);
					common_utility::ChangeNameInfo change_name_info;
					change_name_info.old_name = name;
					change_name_info.new_name = new_name;

					std::replace(change_name_info.old_name.begin(), change_name_info.old_name.end(), L'\\', L'/');  // Window path to Posix path
					std::replace(change_name_info.new_name.begin(), change_name_info.new_name.end(), L'\\', L'/');  // Window path to Posix path
					event_filter->RenameFilter(event_queue, change_name_info);
				}
				else if (FILE_ACTION_MODIFIED == fni->Action) {
					std::optional<bool> is_dir = common_utility::IsDirectory(name);
					if (!is_dir.has_value()) {
						std::wcerr << L"WindowEventPusher::PushEvent: IsDirectory Fail: " << name << std::endl;
						continue;
					}

					if (is_dir.value()) {
						continue;  // 폴더 수정 이벤트는 무시
					}

					std::optional<common_utility::ItemInfo> item_info_opt = common_utility::GetItemInfo(name);
					if (!item_info_opt.has_value()) {
						std::wcerr << L"WindowEventPusher::PushEvent: GetItemInfo Fail: " << name << std::endl;
						continue;
					}

					auto item_info = item_info_opt.value();
					std::replace(item_info.name.begin(), item_info.name.end(), L'\\', L'/');  // Window path to Posix path
					event_filter->UploadFilter(event_queue, item_info);
				}
				else if (FILE_ACTION_REMOVED == fni->Action) {
					std::replace(name.begin(), name.end(), L'\\', L'/');  // Window path to Posix path
					event_filter->RemoveFilter(event_queue, name);
				}
			}
		}
	}

	void WindowEventPusher::PushAddEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (common_utility::HasIgnore(relative_path)) {
			return;
		}

		std::optional<common_utility::ItemInfo> item_info_opt = common_utility::GetItemInfo(relative_path);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"WindowEventPusher::PushAddEvent: GetItemInfo Fail: " << relative_path << std::endl;
			return;
		}

		auto item_info = item_info_opt.value();
		std::replace(item_info.name.begin(), item_info.name.end(), L'\\', L'/');  // Window path to Posix path
		event_filter->UploadFilter(event_queue, item_info);
		if (item_info.size >= 0) {
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
				PushAddEvent(event_filter, event_queue, relative_path_name);

			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}
}  // namespace monitor_client