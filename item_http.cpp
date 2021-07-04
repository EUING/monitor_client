#include "item_http.h"

#include <string>

#include <cpprest/http_client.h>

namespace monitor_client {
	const wchar_t* const kItemEndPoint = U("/item");
	const wchar_t* const kFileEndPoint = U("/file");
	const wchar_t* const kFolderEndPoint = U("/folder");

	ItemHttp::ItemHttp(const common_utility::NetworkInfo& info) : builder_() {
		builder_.set_scheme(U("http")).set_host(info.host).set_port(info.port);
	}

	bool ItemHttp::RenameItem(const common_utility::ChangeNameInfo& name_info) {
		web::json::value patch_data;
		patch_data[U("new_name")] = web::json::value::string(name_info.new_name);

		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/name/"));
		path_variable.append(name_info.old_name);

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::PATCH, U("/"), patch_data).get();
		if (response.status_code() != web::http::status_codes::OK) {
			return false;
		}

		return true;
	}

	bool ItemHttp::RemoveItem(const std::wstring& relative_path) {
		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(relative_path);

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::DEL).get();
		if (response.status_code() != web::http::status_codes::NoContent) {
			return false;
		}

		return true;
	}

	bool ItemHttp::AddFile(const common_utility::FileInfo & info) {
		web::json::value post_data;
		post_data[U("size")] = web::json::value::number(info.size);
		post_data[U("creation_time")] = web::json::value::string(info.creation_time);
		post_data[U("last_modified_time")] = web::json::value::string(info.last_modified_time);

		utility::string_t path_variable = kFileEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(info.name);

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
	    web::http::http_response response = client.request(web::http::methods::POST, U("/"), post_data).get();
		if (response.status_code() != web::http::status_codes::Created) {
			return false;
		}

		return true;
	}

	bool ItemHttp::ModifyFile(const common_utility::FileInfo& info) {		
		web::json::value patch_data;
		patch_data[U("size")] = web::json::value::number(info.size);
		patch_data[U("last_modified_time")] = web::json::value::string(info.last_modified_time);

		utility::string_t path_variable = kFileEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(info.name);

		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::PATCH, U("/"), patch_data).get();
		if (response.status_code() != web::http::status_codes::OK) {
			return false;
		}

		return true;
	}

	bool ItemHttp::AddFolder(const common_utility::FolderInfo& info) {
		web::json::value post_data;
		post_data[U("creation_time")] = web::json::value::string(info.creation_time);

		utility::string_t path_variable = kFolderEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(info.name);
		
		builder_.set_path(path_variable, true);
		web::http::client::http_client client(builder_.to_uri());
		web::http::http_response response = client.request(web::http::methods::POST, U("/"), post_data).get();
		if (response.status_code() != web::http::status_codes::Created) {
			return false;
		}

		return true;
	}
}  // namespace monitor_client