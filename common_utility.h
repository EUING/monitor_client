#ifndef MONITOR_CLIENT_COMMON_UTILITY_H_
#define MONITOR_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>
#include <stdint.h>

#include <memory>
#include <variant>
#include <optional>
#include <string>
#include <vector>

namespace monitor_client {
namespace common_utility {
	struct ChangeItemInfo {
		DWORD action;
		std::wstring relative_path;
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

	struct NetworkInfo {
		std::wstring host;
		int port;
	};
	
	std::optional<std::wstring> ConvertTimestamp(const FILETIME& time);
	std::variant<std::monostate, FileInfo, FolderInfo> GetItemInfo(const std::wstring& relative_path);
	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& relative_path);
	bool SplitPath(const std::wstring& relative_path, std::vector<std::wstring>* split_parent_path, std::wstring& item_name);

	template<typename ... Args>
	std::wstring format_wstring(const std::wstring& format, Args ... args)
	{
		int size = _scwprintf(format.c_str(), args ...) + 1;
		std::unique_ptr<wchar_t[]> buffer = std::make_unique<wchar_t[]>(size);
		_snwprintf_s(buffer.get(), size, size - 1, format.c_str(), args ...);
		return std::wstring(buffer.get(), buffer.get() + size - 1);
	}
}  // namespace common_utility
}  // namespace monitor_client
#endif