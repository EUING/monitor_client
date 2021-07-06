#include "common_utility.h"

#include <Windows.h>
#include <stdint.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>

namespace monitor_client {
namespace common_utility {
	std::optional<bool> IsDirectory(const std::wstring& path) {
		DWORD attribute = GetFileAttributes(path.c_str());
		if (INVALID_FILE_ATTRIBUTES == attribute) {
			std::wcerr << L"common_utility::IsDirectory: GetFileAttributes Fail: " << path << std::endl;
			return std::nullopt;
		}

		return (attribute & FILE_ATTRIBUTE_DIRECTORY);
	}

	std::optional<ItemInfo> GetItemInfo(const std::wstring& relative_path) {
		WIN32_FILE_ATTRIBUTE_DATA file_attribute;
		if (!GetFileAttributesEx(relative_path.c_str(), GetFileExInfoStandard, &file_attribute)) {
			std::wcerr << L"common_utility::GetItemInfo: GetFileAttributesEx Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		int64_t size = -1;
		if (0 == (file_attribute.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			LARGE_INTEGER li;
			li.LowPart = file_attribute.nFileSizeLow;
			li.HighPart = file_attribute.nFileSizeHigh;
			size = li.QuadPart;
		}

		ItemInfo info;
		info.name = relative_path;
		info.size = size;

		return info;
	}

	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& relative_path) {
		std::wistringstream relative_path_stream(relative_path);
		std::vector<std::wstring> string_buffer;
		std::wstring name_path;

		while (std::getline(relative_path_stream, name_path, L'?')) {
			string_buffer.push_back(name_path);
		}

		if (string_buffer.size() != 2) {
			std::wcerr << L"common_utility::SplitChangeName Fail: " << relative_path << std::endl;
			return std::nullopt;
		}

		ChangeNameInfo info;
		info.old_name = string_buffer[0];
		info.new_name = string_buffer[1];

		return info;
	}

	bool SplitPath(const std::wstring& relative_path, std::vector<std::wstring>* split_parent_path, std::wstring& item_name) {
		if (relative_path.empty()) {
			std::wcerr << L"common_utility::SplitPath: relative_path is empty" << std::endl;
			return false;
		}

		wchar_t drive[_MAX_DRIVE] = { 0, };
		wchar_t dir[_MAX_DIR] = { 0, };
		wchar_t fname[_MAX_FNAME] = { 0, };
		wchar_t ext[_MAX_EXT] = { 0, };

		std::wstring path = relative_path;
		std::replace(path.begin(), path.end(), L'\\', L'/');  // Window path to Posix path

		errno_t result = _wsplitpath_s(path.c_str(), drive, dir, fname, ext);
		if (0 != result) {
			std::wcerr << L"common_utility::SplitPath: _wsplitpath_s Fail: " << path << std::endl;
			return false;
		}

		item_name = fname;
		item_name.append(ext);

		if (split_parent_path) {
			split_parent_path->clear();

			std::wstring parent_folder = dir;
			std::wistringstream path_stream(parent_folder);
			std::wstring name_path;

			while (std::getline(path_stream, name_path, L'/')) {
				split_parent_path->push_back(name_path);
			}
		}

		return true;
	}
}  // namespace common_utility
}  // namespace monitor_client