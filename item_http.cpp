#include "item_http.h"

#include <iostream>
#include <string>

#include <cpprest/http_client.h>

namespace monitor_client {
	const wchar_t* const kItemEndPoint = U("/item");

	ItemHttp::ItemHttp(const common_utility::NetworkInfo& info) : builder_() {
		builder_.set_scheme(U("http")).set_host(info.host).set_port(info.port);
	}

	std::optional<common_utility::ItemInfo> ItemHttp::GetItemInfo(const std::wstring& relative_path /*= L""*/) const {
		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(relative_path);

		web::http::uri_builder builder(builder_);
		builder.set_path(path_variable, true);
		web::http::client::http_client client(builder.to_uri());
		web::http::http_response response = client.request(web::http::methods::GET).get();
		if (response.status_code() == web::http::status_codes::OK) {
			web::json::value json_object = response.extract_json().get();
			if (!json_object.is_object()) {
				std::wcerr << L"ItemHttp::GetItemInfo: wrong item info" << std::endl;
				return std::nullopt;
			}

			common_utility::ItemInfo info;
			web::json::object object = json_object.as_object();
			info.name = object[U("name")].as_string();
			info.size = object[U("size")].as_integer();
			info.hash = object[U("hash")].as_string();

			return info;
		}

		std::wcerr << L"ItemHttp::GetItemInfo: request Fail" << std::endl;
		return std::nullopt;
	}

	std::optional<std::vector<common_utility::ItemInfo>> ItemHttp::GetFolderContainList(const std::wstring& relative_path /*= L""*/) const {
		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/contain/"));
		if (relative_path.empty()) {
			path_variable.pop_back();
		}
		else {
			path_variable.append(relative_path);
		}

		web::http::uri_builder builder(builder_);
		builder.set_path(path_variable, true);

		web::http::client::http_client client(builder.to_uri());
		web::http::http_response response = client.request(web::http::methods::GET).get();
		if (response.status_code() == web::http::status_codes::OK) {
			web::json::value json_object = response.extract_json().get();
			if (!json_object.is_array()) {
				return std::nullopt;
			}

			std::vector<common_utility::ItemInfo> item_list;
			web::json::array arr = json_object.as_array();
			for (auto iter : arr) {
				if (!iter.is_object()) {
					return std::nullopt;
				}

				common_utility::ItemInfo info;
				web::json::object object = iter.as_object();
				info.name = object[U("name")].as_string();
				info.size = object[U("size")].as_integer();
				info.hash = object[U("hash")].as_string();

				item_list.push_back(info);
			}

			return item_list;
		}

		std::wcerr << L"ItemHttp::GetFolderContainList: request Fail" << std::endl;
		return std::nullopt;
	}

	bool ItemHttp::InsertItem(const common_utility::ItemInfo& item_info) {
		web::json::value put_data;
		put_data[U("size")] = web::json::value::number(item_info.size);
		put_data[U("hash")] = web::json::value::string(item_info.hash);

		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(item_info.name);

		web::http::uri_builder builder(builder_);
		builder.set_path(path_variable, true);
		web::http::client::http_client client(builder.to_uri());
		web::http::http_response response = client.request(web::http::methods::PUT, U("/"), put_data).get();
		switch (response.status_code()) {
		case web::http::status_codes::OK:
		case web::http::status_codes::Created:
			return true;
		}

		std::wcerr << L"ItemHttp::InsertItem: request Fail" << std::endl;
		return false;
	}

	bool ItemHttp::RenameItem(const common_utility::ChangeNameInfo& name_info) {
		web::json::value patch_data;
		patch_data[U("new_name")] = web::json::value::string(name_info.new_name);

		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(name_info.old_name);

		web::http::uri_builder builder(builder_);
		builder.set_path(path_variable, true);
		web::http::client::http_client client(builder.to_uri());
		web::http::http_response response = client.request(web::http::methods::PATCH, U("/"), patch_data).get();
		if (response.status_code() == web::http::status_codes::OK) {
			return true;
		}

		std::wcerr << L"ItemHttp::RenameItem: request Fail" << std::endl;
		return false;
	}

	bool ItemHttp::RemoveItem(const std::wstring& relative_path) {
		utility::string_t path_variable = kItemEndPoint;
		path_variable.append(U("/info/"));
		path_variable.append(relative_path);

		web::http::uri_builder builder(builder_);
		builder.set_path(path_variable, true);
		web::http::client::http_client client(builder.to_uri());
		web::http::http_response response = client.request(web::http::methods::DEL).get();
		switch (response.status_code()) {
		case web::http::status_codes::OK:
		case web::http::status_codes::NoContent:
			return true;
		}

		std::wcerr << L"ItemHttp::RemoveItem: request Fail" << std::endl;
		return false;
	}
}  // namespace monitor_client