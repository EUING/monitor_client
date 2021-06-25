#ifndef MONITOR_CLIENT_ITEM_HTTP_H_
#define MONITOR_CLIENT_ITEM_HTTP_H_

#include <optional>
#include <vector>
#include <string>

#include <cpprest/http_client.h>

#include "common_utility.h"

namespace monitor_client {
	class ItemHttp {
	public:
		ItemHttp(const std::wstring& host, int port);

		ItemHttp(const ItemHttp&) = default;
		ItemHttp& operator=(const ItemHttp&) = default;

		ItemHttp(ItemHttp&&) = default;
		ItemHttp& operator=(ItemHttp&&) = default;

		~ItemHttp() = default;

		std::wstring GetHost() { return builder_.host(); }
		void SetHost(const std::wstring& host) { builder_.set_host(host); }

		int GetPort() { return builder_.port(); }
		void SetPort(int port) { builder_.set_port(port); }

		std::optional<std::vector<common_utility::FileInfo>> GetTotalFile();
		std::optional<common_utility::FileInfo> GetFile(const std::wstring& file_name);
		bool AddFolder(const common_utility::FolderInfo& info);
		bool AddFile(const common_utility::FileInfo& info);
		bool ModifyFile(const common_utility::FileInfo& info);
		bool RemoveFile(const std::wstring& file_name);
		bool RenameFile(const common_utility::ChangeNameInfo& name_info);

	private:
		web::http::uri_builder builder_;
	};
}  // namespace monitor_client
#endif
