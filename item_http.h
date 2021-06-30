#ifndef MONITOR_CLIENT_ITEM_HTTP_H_
#define MONITOR_CLIENT_ITEM_HTTP_H_

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

		bool RenameItem(const common_utility::ChangeNameInfo& name_info);
		bool RemoveItem(const std::wstring& itme_name);

		bool AddFile(const common_utility::FileInfo& info);
		bool ModifyFile(const common_utility::FileInfo& info);
		
		bool AddFolder(const common_utility::FolderInfo& info);

	private:
		web::http::uri_builder builder_;
	};
}  // namespace monitor_client
#endif
