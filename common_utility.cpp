#include "common_utility.h"

#include <Windows.h>
#include <atlstr.h>
#include <stdint.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>

#include "sha256.h"

namespace monitor_client {
namespace common_utility {
	bool operator==(const ItemInfo& lhs, const ItemInfo& rhs) {
		return lhs.name == rhs.name;
	}

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
		std::wstring hash;
		if (0 == (file_attribute.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			std::optional<std::wstring> sha256 = GetSha256(relative_path);
			if (!sha256.has_value()) {
				std::wcerr << L"common_utility::GetItemInfo: GetSha256 Fail: " << relative_path << std::endl;
				return std::nullopt;
			}

			LARGE_INTEGER li;
			li.LowPart = file_attribute.nFileSizeLow;
			li.HighPart = file_attribute.nFileSizeHigh;
			size = li.QuadPart;
			hash = sha256.value();
		}

		ItemInfo info;
		info.name = relative_path;
		info.size = size;
		info.hash = hash;

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

	std::optional<std::wstring> GetSha256(std::wstring file_path) {
		std::ifstream is(file_path, std::ifstream::binary);
		if (!is) {
			return std::nullopt;
		}
		
		is.seekg(0, std::ifstream::end);
		int length = (int)is.tellg();
		is.seekg(0, std::ifstream::beg);

		std::vector<unsigned char> buffer(length, 0);
		auto raw_pointer = buffer.data();
		is.read(reinterpret_cast<char*>(raw_pointer), length);
		is.close();

		unsigned char digest[SHA256::DIGEST_SIZE] = { 0, };
			
		SHA256 ctx;
		ctx.init();
		ctx.update(raw_pointer, length);
		ctx.final(digest);

		char buf[2 * SHA256::DIGEST_SIZE + 1] = { 0, };
		for (int i = 0; i < SHA256::DIGEST_SIZE; i++) {
			sprintf_s(buf + (i * 2), sizeof(buf) - (i * 2), "%02x", digest[i]);
		}

		return CA2W(buf).m_psz;
	}
}  // namespace common_utility
}  // namespace monitor_client