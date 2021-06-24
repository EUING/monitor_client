#include "file_http.h"

#include <optional>
#include <string>

#include <cpprest/http_client.h>

namespace monitor_client {
	const wchar_t* const kEndPoint = U("/files");

	FileHttp::FileHttp(const std::wstring& host, int port) : builder_() {
		builder_.set_scheme(U("http")).set_host(host).set_port(port);
	}

	bool FileHttp::AddFile(const common_utility::FileInfo & info) {
		web::json::value post_data;
		post_data[U("name")] = web::json::value::string(info.file_name);
		post_data[U("size")] = web::json::value::string(info.file_size);
		post_data[U("creation_time")] = web::json::value::string(info.creation_iso_time);
		post_data[U("last_modified_time")] = web::json::value::string(info.last_modified_iso_time);

		builder_.set_path(kEndPoint);
		web::http::client::http_client client(builder_.to_uri());
	    web::http::http_response response = client.request(web::http::methods::POST, U("/"), post_data).get();
		if (response.status_code() != web::http::status_codes::Created) {
			return false;
		}

		return true;
	}

	bool FileHttp::ModifyFile(const common_utility::FileInfo& info) {		
		web::json::value patch_data;
		patch_data[U("name")] = web::json::value::string(info.file_name);
		patch_data[U("size")] = web::json::value::string(info.file_size);
		patch_data[U("last_modified_time")] = web::json::value::string(info.last_modified_iso_time);

		utility::string_t path_variable = kEndPoint;
		path_variable.push_back(U('/'));
		path_variable.append(info.file_name);

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::PATCH, U("/"), patch_data).get();
		if (response.status_code() != web::http::status_codes::OK) {
			return false;
		}

		return true;
	}

	bool FileHttp::RemoveFile(const std::wstring& file_name) {
		utility::string_t path_variable = kEndPoint;
		path_variable.push_back(U('/'));
		path_variable.append(file_name);

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::DEL).get();
		if (response.status_code() != web::http::status_codes::Accepted) {
			return false;
		}

		return true;
	}

	bool FileHttp::RenameFile(const common_utility::ChangeNameInfo& name_info) {
		web::json::value patch_data;
		patch_data[U("old_name")] = web::json::value::string(name_info.old_name);
		patch_data[U("new_name")] = web::json::value::string(name_info.new_name);

		utility::string_t path_variable = kEndPoint;
		path_variable.push_back(U('/'));
		path_variable.append(name_info.old_name);
		path_variable.append(U("/name"));

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::PATCH, U("/"), patch_data).get();
		if (response.status_code() != web::http::status_codes::OK) {
			return false;
		}

		return true;
	}
}  // namespace monitor_client