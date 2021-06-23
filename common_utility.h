#ifndef REST_CLIENT_COMMON_UTILITY_H_
#define REST_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>

#include <optional>
#include <string>

namespace my_rest_client {
namespace common_utility {
	struct ChangeObjectInfo {
		DWORD action;
		std::wstring full_path;
	};

	struct ChangeNameInfo {
		std::wstring old_name_path;
		std::wstring new_name_path;
	};

	struct FileInfo {
		std::wstring file_name;
		std::wstring creation_iso_time;
		std::wstring last_modified_iso_time;
		std::wstring file_size;
	};
	
	std::optional<std::wstring> ConvertIsoTime(const FILETIME& time);
	std::optional<FileInfo> GetFileInfo(const std::wstring& file_name);
	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& full_path);
}  // namespace common_utility
}  // namespace my_rest_client
#endif