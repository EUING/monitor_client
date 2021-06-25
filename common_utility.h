#ifndef MONITOR_CLIENT_COMMON_UTILITY_H_
#define MONITOR_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>
#include <stdint.h>

#include <variant>
#include <optional>
#include <string>

namespace monitor_client {
namespace common_utility {
	struct ChangeItemInfo {
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

	struct FolderInfo {
		std::wstring name;
		std::wstring creation_time;
	};
	
	std::optional<std::wstring> GetItemName(const std::wstring& full_path);
	std::optional<std::wstring> ConvertTimestamp(const FILETIME& time);
	std::variant<std::monostate, FileInfo, FolderInfo> GetItemInfo(const std::wstring& full_path);
	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& full_path);
}  // namespace common_utility
}  // namespace monitor_client
#endif