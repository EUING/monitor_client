#include "common_utility.h"

#include <Windows.h>
#include <atlstr.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <memory>
#include <thread>

#include "sha256.h"

namespace monitor_client {
namespace common_utility {
	void GetSubFolderInfo(const std::wstring& folder_path, ItemList& item_list) {
		WIN32_FIND_DATA find_data;
		std::wstring fname = folder_path + L"/*.*";

		HANDLE handle = FindFirstFile(fname.c_str(), &find_data);
		if (INVALID_HANDLE_VALUE != handle) {
			do {
				std::wstring file_name(find_data.cFileName);
				if (L"." == file_name || L".." == file_name) {
					continue;
				}

				std::wstring relative_path_name = (folder_path == L".") || (folder_path == L"..") ? file_name : folder_path + L'/' + file_name;
				std::optional<ItemInfo> item_info = GetItemInfo(relative_path_name);
				if (!item_info.has_value()) {
					std::wcerr << L"common_utility::GetSubFolderInfo: GetItemInfo Fail: " << relative_path_name << std::endl;
					return;
				}

				item_list.insert(item_info.value());

				GetSubFolderInfo(relative_path_name, item_list);
			} while (FindNextFile(handle, &find_data));
		}

		FindClose(handle);
	}

	void GetSubFolderInfo(const LocalDb& local_db, const std::wstring& folder_path, ItemList& item_list) {
		std::optional<std::vector<common_utility::ItemInfo>> sub_item = local_db.GetFolderContainList(folder_path);
		if (!sub_item.has_value()) {
			std::wcerr << L"common_utility::GetSubFolderInfo: local_db.GetFolderContainList Fail: " << folder_path << std::endl;
			return;
		}

		auto sub_item_list = sub_item.value();
		for (const auto& item : sub_item_list) {
			std::wstring relative_path = folder_path.empty() ? item.name : folder_path + L'/' + item.name;
			item_list.insert({ relative_path, item.size, item.hash });

			if (item.size < 0) {
				GetSubFolderInfo(local_db, relative_path, item_list);
			}
		}
	}

	void GetSubFolderInfo(const ItemHttp& item_http, const std::wstring& folder_path, ItemList& item_list) {
		std::optional<std::vector<common_utility::ItemInfo>> sub_item = item_http.GetFolderContainList(folder_path);
		if (!sub_item.has_value()) {
			std::wcerr << L"common_utility::GetSubFolderInfo: item_http.GetFolderContainList Fail: " << folder_path << std::endl;
			return;
		}

		auto sub_item_list = sub_item.value();
		for (const auto& item : sub_item_list) {
			std::wstring relative_path = folder_path.empty() ? item.name : folder_path + L'/' + item.name;
			item_list.insert({ relative_path, item.size, item.hash });

			if (item.size < 0) {
				GetSubFolderInfo(item_http, relative_path, item_list);
			}
		}
	}

	bool WaitTimeForAccess(const std::wstring& relative_path, int time /*= 1000 ms*/) {
		auto invalid_deleter = [](HANDLE handle) {
			if (INVALID_HANDLE_VALUE != handle)
				CloseHandle(handle);
		};

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		while (true) {
			std::unique_ptr<void, decltype((invalid_deleter))> handle_ptr(nullptr, invalid_deleter);
			HANDLE h = CreateFile(relative_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
			handle_ptr.reset(h);

			if (handle_ptr.get() != INVALID_HANDLE_VALUE) {
				return true;
			}
			else if (GetLastError() != ERROR_SHARING_VIOLATION) {
				std::wcerr << L"common_utility::WaitTimeForAccess: CreateFile Fail: " << relative_path << std::endl;
				break;
			}
			else {
				std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
				if (sec.count() * 1000 > time) {
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}

		return false;  // time_out or CreateFile Error
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

	std::optional<bool> IsDirectory(const std::wstring& path) {
		DWORD attribute = GetFileAttributes(path.c_str());
		if (INVALID_FILE_ATTRIBUTES == attribute) {
			std::wcerr << L"common_utility::IsDirectory: GetFileAttributes Fail: " << path << std::endl;
			return std::nullopt;
		}

		return (attribute & FILE_ATTRIBUTE_DIRECTORY);
	}

	std::optional<bool> IsValidItem(const ItemInfo& item_info) {
		std::optional<ItemInfo> get_item = GetItemInfo(item_info.name);
		if (!get_item.has_value()) {
			return std::nullopt;
		}

		auto current_item_info = get_item.value();

		if (current_item_info.hash != item_info.hash) {
			return false;
		}

		return true;
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

	bool HasIgnore(const std::wstring& relative_path) {
		return std::wstring::npos != relative_path.find(L".ignore");
	}

	bool HasConflict(const std::wstring& relative_path) {
		return std::wstring::npos != relative_path.find(L".conflict");
	}
	
}  // namespace common_utility
}  // namespace monitor_client