#include "window_event_pusher.h"

#include <Windows.h>
#include <stdint.h>

#include <string>
#include <algorithm>
#include <optional>
#include <memory>

#include "common_struct.h"
#include "common_utility.h"
#include "event_queue.h"
#include "upload_event.h"
#include "remove_event.h"
#include "rename_event.h"

namespace monitor_client {
	void WindowEventPusher::PushEvent(std::shared_ptr<EventQueue> event_queue) const {
		const FILE_NOTIFY_INFORMATION* fni = nullptr;
		bool is_first = true;

		for (DWORD offset = 0; is_first || fni->NextEntryOffset != 0; offset += fni->NextEntryOffset) {
			is_first = false;
			fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&buffer_[offset]);

			if (FILE_ACTION_ADDED == fni->Action) {
				std::wstring name(fni->FileName, fni->FileNameLength / 2);
				PushAddEvent(event_queue, name);
				continue;
			}
			else {
				if (FILE_ACTION_RENAMED_OLD_NAME == fni->Action) {
					std::wstring old_name(fni->FileName, fni->FileNameLength / 2);

					offset += fni->NextEntryOffset;
					fni = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&buffer_[offset]);  // NEW_NAME을 알기 위해 offset을 증가

					std::wstring new_name(fni->FileName, fni->FileNameLength / 2);
					common_utility::ChangeNameInfo change_name_info;
					change_name_info.old_name = old_name;
					change_name_info.new_name = new_name;

					std::replace(change_name_info.old_name.begin(), change_name_info.old_name.end(), L'\\', L'/');  // Window path to Posix path
					std::replace(change_name_info.new_name.begin(), change_name_info.new_name.end(), L'\\', L'/');  // Window path to Posix path
					std::wclog << L"WindowEventPusher RenameEvent: " << change_name_info.old_name << std::endl;
					event_queue->Push(std::make_unique<RenameEvent>(change_name_info));
				}
				else if (FILE_ACTION_MODIFIED == fni->Action) {
					std::wstring name(fni->FileName, fni->FileNameLength / 2);
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
					std::wclog << L"WindowEventPusher UploadEvent: " << item_info.name << std::endl;
					event_queue->Push(std::make_unique<UploadEvent>(item_info));
				}
				else if (FILE_ACTION_REMOVED == fni->Action) {
					std::wstring name(fni->FileName, fni->FileNameLength / 2);
					std::replace(name.begin(), name.end(), L'\\', L'/');  // Window path to Posix path
					std::wclog << L"WindowEventPusher RemoveEvent: " << name << std::endl;
					event_queue->Push(std::make_unique<RemoveEvent>(name));
				}
			}
		}
	}

	void WindowEventPusher::PushAddEvent(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		std::optional<common_utility::ItemInfo> item_info_opt = common_utility::GetItemInfo(relative_path);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"WindowEventPusher::PushAddEvent: GetItemInfo Fail: " << relative_path << std::endl;
			return;
		}

		auto item_info = item_info_opt.value();
		std::replace(item_info.name.begin(), item_info.name.end(), L'\\', L'/');  // Window path to Posix path
		std::wclog << L"WindowEventPusher UploadEvent: " << item_info.name << std::endl;
		event_queue->Push(std::make_unique<UploadEvent>(item_info));
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
				PushAddEvent(event_queue, relative_path_name);

			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}
}  // namespace monitor_client