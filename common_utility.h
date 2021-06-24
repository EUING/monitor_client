#ifndef REST_CLIENT_COMMON_UTILITY_H_
#define REST_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>
#include <stdint.h>

#include <optional>
#include <string>

namespace monitor_client {
namespace common_utility {
	struct ChangeObjectInfo {
		DWORD action;
		std::wstring full_path;
	};

	struct ChangeNameInfo {
		std::wstring old_name;
		std::wstring new_name;
	};

	struct FileInfo {
		std::wstring name;
		int64_t size;
		std::wstring creation_time;
		std::wstring last_modified_time;
	};
	
	std::optional<std::wstring> GetFileName(const std::wstring& full_path);
	std::optional<std::wstring> ConvertTimestamp(const FILETIME& time);
	std::optional<FileInfo> GetFileInfo(const std::wstring& full_path);
	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& full_path);
}  // namespace common_utility
}  // namespace monitor_client
#endif