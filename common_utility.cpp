#include "common_utility.h"

#include <Windows.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace my_rest_client {
namespace common_utility {
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

	std::optional<FileInfo> GetFileInfo(const std::wstring& file_name) {
		WIN32_FILE_ATTRIBUTE_DATA file_attribute;
		if (!GetFileAttributesEx(file_name.c_str(), GetFileExInfoStandard, &file_attribute)) {
			return std::nullopt;
		}

		if (0 != (file_attribute.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			return std::nullopt;  // 폴더는 제외
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
		info.file_name = file_name;
		info.file_size = std::to_wstring(li.QuadPart);
		info.creation_iso_time = creation_result.value();
		info.last_modified_iso_time = write_result.value();

		return info;
	}

	std::optional<ChangeNameInfo> SplitChangeName(const std::wstring& full_path) {
		std::wistringstream full_path_stream(full_path);
		std::vector<std::wstring> string_buffer;
		std::wstring name_path;

		while (std::getline(full_path_stream, name_path, L':')) {
			string_buffer.push_back(name_path);
		}

		if (string_buffer.size() != 2) {
			return std::nullopt;
		}

		ChangeNameInfo info;
		info.old_name_path = string_buffer[0];
		info.new_name_path = string_buffer[1];

		return info;
	}

}  // namespace common_utility
}  // namespace my_rest_client