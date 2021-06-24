#include "common_utility.h"

#include <Windows.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace monitor_client {
namespace common_utility {
	std::optional<std::wstring> GetFileName(const std::wstring& full_path) {
		wchar_t drive[_MAX_DRIVE] = { 0, };
		wchar_t dir[_MAX_DIR] = { 0, };
		wchar_t fname[_MAX_FNAME] = { 0, };
		wchar_t ext[_MAX_EXT] = { 0, };

		errno_t result = _wsplitpath_s(full_path.c_str(), drive, dir, fname, ext);
		if (0 != result) {
			return std::nullopt;
		}

		std::wstring extension(ext);
		if (extension.empty()) {
			return std::nullopt;
		}

		return (fname + extension);
	}

	std::optional<std::wstring> ConvertIsoTime(const FILETIME& time) {
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
		stream << std::setfill(L'0') << std::setw(4) << local_time.wYear << L'-' << w2 << local_time.wMonth << L'-' << w2 << local_time.wDay << L'T';
		stream << w2 << local_time.wHour << L':' << w2 << local_time.wMinute << L':' << w2 << local_time.wSecond;

		return stream.str();
	}

	std::optional<FileInfo> GetFileInfo(const std::wstring& full_path) {
		WIN32_FILE_ATTRIBUTE_DATA file_attribute;
		if (!GetFileAttributesEx(full_path.c_str(), GetFileExInfoStandard, &file_attribute)) {
			return std::nullopt;
		}

		if (0 != (file_attribute.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			return std::nullopt;  // 폴더는 제외
		}

		std::optional<std::wstring> file_name_result = GetFileName(full_path);
		if (!file_name_result.has_value()) {
			return std::nullopt;
		}

		std::optional<std::wstring> creation_result = ConvertIsoTime(file_attribute.ftCreationTime);
		if (!creation_result.has_value()) {
			return std::nullopt;
		}

		std::optional<std::wstring> write_result = ConvertIsoTime(file_attribute.ftLastWriteTime);
		if (!write_result.has_value()) {
			return std::nullopt;
		}

		ULARGE_INTEGER li;
		li.LowPart = file_attribute.nFileSizeLow;
		li.HighPart = file_attribute.nFileSizeHigh;

		FileInfo info;
		info.file_name = file_name_result.value();
		info.file_size = std::to_wstring(li.QuadPart);
		info.creation_iso_time = creation_result.value();
		info.last_modified_iso_time = write_result.value();

		return info;
	}

	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& full_path) {
		std::wistringstream full_path_stream(full_path);
		std::vector<std::wstring> string_buffer;
		std::wstring name_path;

		while (std::getline(full_path_stream, name_path, L'?')) {
			string_buffer.push_back(name_path);
		}

		if (string_buffer.size() != 2) {
			return std::nullopt;
		}

		std::optional<std::wstring> old_name_result = GetFileName(string_buffer[0]);
		if (!old_name_result.has_value()) {
			return std::nullopt;
		}

		std::optional<std::wstring> new_name_result = GetFileName(string_buffer[1]);
		if (!new_name_result.has_value()) {
			return std::nullopt;
		}

		ChangeNameInfo info;
		info.old_name = old_name_result.value();
		info.new_name = new_name_result.value();

		return info;
	}

}  // namespace common_utility
}  // namespace monitor_client