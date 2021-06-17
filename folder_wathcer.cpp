#include "folder_watcher.h"

#include <Windows.h>

#include <array>
#include <memory>
#include <string>
#include <optional>

namespace my_rest_client {
	bool g_watching = true;

	std::optional<bool> WatchDirectory(const std::wstring& folder_name) {
		const wchar_t* dir = folder_name.c_str();

		std::array<HANDLE, 2> raw_handles = { 0, };
		raw_handles[0] = FindFirstChangeNotification(dir, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
		if (INVALID_HANDLE_VALUE == raw_handles[0]) {
			return std::nullopt;
		}

		raw_handles[1] = FindFirstChangeNotification(dir, TRUE, FILE_NOTIFY_CHANGE_DIR_NAME);
		if (INVALID_HANDLE_VALUE == raw_handles[1]) {
			return std::nullopt;
		}
		
		auto deleter = [](HANDLE* handle) {
			if (*handle) {
				FindCloseChangeNotification(*handle);
			}
		};

		std::unique_ptr<HANDLE, decltype(deleter)> file_handle(std::move(&raw_handles[0]), deleter);
		std::unique_ptr<HANDLE, decltype(deleter)> dir_handle(std::move(&raw_handles[1]), deleter);

		const HANDLE change_handles[2] = { *file_handle, *dir_handle };
		DWORD wait_status = 0;

		bool result = false;
		while (g_watching) {
			wait_status = WaitForMultipleObjects(2, change_handles, FALSE, 5000);

			switch (wait_status) {
			case WAIT_OBJECT_0: {
				// file 생성, 이름 변경, 제거 된 경우
				result = true;
				FindNextChangeNotification(change_handles[0]);
				break;
			}
			case WAIT_OBJECT_0 + 1: {
				// folder 생성, 이름 변경, 제거 된 경우
				result = true;
				FindNextChangeNotification(change_handles[1]);
				break;
			}
			case WAIT_TIMEOUT:
				continue;
			default:
				return std::nullopt;
			}	
		}

		return result;
	}
}