#ifndef REST_CLIENT_COMMON_UTILITY_H_
#define REST_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>

#include <optional>
#include <string>

namespace my_rest_client {
namespace common_utility {
	struct ChangeInfo {
		DWORD action;
		std::wstring full_path;
	};

	struct FileInfo {
		std::wstring file_name;
		std::wstring creation_iso_time;
		std::wstring last_modified_iso_time;
		std::wstring file_size;
	};
	
	std::optional<FileInfo> GetFileInfo(const std::wstring& file_name);
	std::optional<std::wstring> ConvertIsoTime(const FILETIME& time);
}  // namespace common_utility
}  // namespace my_rest_client
#endif