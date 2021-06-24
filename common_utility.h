#ifndef REST_CLIENT_COMMON_UTILITY_H_
#define REST_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>

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
		std::wstring file_name;
		std::wstring file_size;
		std::wstring creation_iso_time;
		std::wstring last_modified_iso_time;
	};
	
	std::optional<std::wstring> GetFileName(const std::wstring& full_path);
	std::optional<std::wstring> ConvertIsoTime(const FILETIME& time);
	std::optional<FileInfo> GetFileInfo(const std::wstring& full_path);
	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& full_path);
}  // namespace common_utility
}  // namespace monitor_client
#endif