#include "common_utility.h"

#include <Windows.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>

namespace monitor_client {
namespace common_utility {
	std::optional<std::wstring> ConvertTimestamp(const FILETIME& time) {
		SYSTEMTIME utc;
		if (!FileTimeToSystemTime(&time, &utc)) {
			return std::nullopt;
		}

		SYSTEMTIME local_time;
		if (!SystemTimeToTzSpecificLocalTime(NULL, &utc, &local_time)) {
			return std::nullopt;
		}

		std::wostringstream stream;
		const auto w2 = std::setw(2);
		stream << std::setfill(L'0') << std::setw(4) << local_time.wYear << L'-' << w2 << local_time.wMonth << L'-' << w2 << local_time.wDay << L' ';
		stream << w2 << local_time.wHour << L':' << w2 << local_time.wMinute << L':' << w2 << local_time.wSecond;

		return stream.str();
	}

	std::variant<std::monostate, FileInfo, FolderInfo> GetItemInfo(const std::wstring& relative_path) {
		WIN32_FILE_ATTRIBUTE_DATA file_attribute;
		if (!GetFileAttributesEx(relative_path.c_str(), GetFileExInfoStandard, &file_attribute)) {
			return {};
		}

		std::optional<std::wstring> creation_result = ConvertTimestamp(file_attribute.ftCreationTime);
		if (!creation_result.has_value()) {
			return {};
		}

		if (0 != (file_attribute.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			FolderInfo info;
			info.name = relative_path;
			info.creation_time = creation_result.value();
			return info;
		}
		else {
			std::optional<std::wstring> write_result = ConvertTimestamp(file_attribute.ftLastWriteTime);
			if (!write_result.has_value()) {
				return {};
			}

			ULARGE_INTEGER li;
			li.LowPart = file_attribute.nFileSizeLow;
			li.HighPart = file_attribute.nFileSizeHigh;

			FileInfo info;
			info.name = relative_path;
			info.size = li.QuadPart;
			info.creation_time = creation_result.value();
			info.last_modified_time = write_result.value();

			return info;
		}
	}

	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& relative_path) {
		std::wistringstream relative_path_stream(relative_path);
		std::vector<std::wstring> string_buffer;
		std::wstring name_path;

		while (std::getline(relative_path_stream, name_path, L'?')) {
			string_buffer.push_back(name_path);
		}

		if (string_buffer.size() != 2) {
			return std::nullopt;
		}

		ChangeNameInfo info;
		info.old_name = string_buffer[0];
		info.new_name = string_buffer[1];

		return info;
	}

	bool SplitPath(const std::wstring& relative_path, std::vector<std::wstring>& split_parent_path, std::wstring& item_name) {
		if (relative_path.empty()) {
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
			return false;
		}

		split_parent_path.clear();

		std::wstring parent_folder = dir;
		std::wistringstream path_stream(parent_folder);
		std::wstring name_path;

		while (std::getline(path_stream, name_path, L'/')) {
			split_parent_path.push_back(name_path);
		}

		item_name = fname;
		item_name.append(ext);

		return true;
	}
}  // namespace common_utility
}  // namespace monitor_client