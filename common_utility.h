#ifndef MONITOR_CLIENT_COMMON_UTILITY_H_
#define MONITOR_CLIENT_COMMON_UTILITY_H_

#include <unordered_set>
#include <optional>
#include <vector>
#include <memory>

#include "common_struct.h"
#include "local_db.h"
#include "item_http.h"

namespace monitor_client {
namespace common_utility {	
	using ItemList = std::unordered_set<common_utility::ItemInfo>;
	void GetSubFolderInfo(const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const LocalDb& local_db, const std::wstring& folder_path, ItemList& item_list);
	void GetSubFolderInfo(const ItemHttp& item_http, const std::wstring& folder_path, ItemList& item_list);

	bool WaitTimeForAccess(const std::wstring& relative_path, int time = 1000 /*ms*/);
	std::optional<ItemInfo> GetItemInfo(const std::wstring& relative_path);
	std::optional<std::wstring> GetSha256(std::wstring file_path);
	std::optional<bool> IsDirectory(const std::wstring& path);
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