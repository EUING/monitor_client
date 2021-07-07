#ifndef MONITOR_CLIENT_COMMON_UTILITY_H_
#define MONITOR_CLIENT_COMMON_UTILITY_H_

#include <Windows.h>
#include <stdint.h>

#include <memory>
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

	struct ItemInfo {
		std::wstring name;
		int64_t size;
		std::wstring hash;
	};

	struct NetworkInfo {
		std::wstring host;
		int port;
	};
	
	std::optional<bool> IsDirectory(const std::wstring& path);
	std::optional<ItemInfo> GetItemInfo(const std::wstring& relative_path);
	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& relative_path);
	bool SplitPath(const std::wstring& relative_path, std::vector<std::wstring>* split_parent_path, std::wstring& item_name);
	std::optional<std::wstring> GetSha256(std::wstring file_path);

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