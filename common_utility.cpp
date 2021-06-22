#include "common_utility.h"

#include <Windows.h>

#include <iomanip>
#include <sstream>

namespace my_rest_client {
namespace common_utility {
	std::optional<FileInfo> GetFileInfo(const std::wstring& file_name) {
		WIN32_FILE_ATTRIBUTE_DATA file_attribute;
		if (!GetFileAttributesEx(file_name.c_str(), GetFileExInfoStandard, &file_attribute)) {
			return std::nullopt;
		}

		if (0 != (file_attribute.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			return std::nullopt;  // ������ ����
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
}  // namespace common_utility
}  // namespace my_rest_client