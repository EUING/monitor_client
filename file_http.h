#ifndef REST_CLIENT_FILE_HTTP_H_
#define REST_CLIENT_FILE_HTTP_H_

#include <string>

#include <cpprest/http_client.h>

#include "common_utility.h"

namespace monitor_client {
	class FileHttp {
	public:
		FileHttp(const std::wstring& host, int port);

		FileHttp(const FileHttp&) = default;
		FileHttp& operator=(const FileHttp&) = default;

		FileHttp(FileHttp&&) = default;
		FileHttp& operator=(FileHttp&&) = default;

		~FileHttp() = default;

		std::wstring GetHost() { return builder_.host(); }
		void SetHost(const std::wstring& host) { builder_.set_host(host); }

		int GetPort() { return builder_.port(); }
		void SetPort(int port) { builder_.set_port(port); }

		bool AddFile(const common_utility::FileInfo& info);
		bool ModifyFile(const common_utility::FileInfo& info);
		bool RemoveFile(const std::wstring& file_name);
		bool RenameFile(const common_utility::ChangeNameInfo& name_info);

	private:
		web::http::uri_builder builder_;
	};
}  // namespace monitor_client
#endif
