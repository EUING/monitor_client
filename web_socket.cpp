#include "web_socket.h"

#include <iostream>
#include <string>

#include <cpprest/uri_builder.h>
#include <cpprest/json.h>

#include "common_struct.h"

namespace monitor_client {
	WebSocket::WebSocket(const common_utility::NetworkInfo& info) : builder_(), client_(), is_connected_(false) {
		builder_.set_scheme(U("ws")).set_host(info.host).set_port(info.port);
	}

	WebSocket::~WebSocket() {
		Close();
	}

	bool WebSocket::Connect() {
		Concurrency::task_status status = client_.connect(builder_.to_uri()).wait();
		if (Concurrency::task_status::completed != status) {
			std::wcerr << L"WebSocket::Connect: client_.connect fail: " << builder_.to_string() << std::endl;
			return false;
		}

		client_.set_close_handler([this](web::websockets::client::websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error) {
			is_connected_ = false;
			});

		client_.set_message_handler([](web::websockets::client::websocket_incoming_message msg) {
			Concurrency::task<std::string> message = msg.extract_string();
			web::json::value json_object = web::json::value::parse(message.get());
			web::json::object object = json_object.as_object();

			std::wstring event = object[U("event")].as_string();
			if (L"download" == event) {
				std::wstring name = object[U("name")].as_string();
				int size = object[U("size")].as_integer();
				std::wstring hash = object[U("hash")].as_string();
			}
			else if (L"rename" == event) {
				std::wstring old_name = object[U("old_name")].as_string();
				std::wstring new_name = object[U("new_name")].as_string();
			}
			else if (L"remove" == event) {
				std::wstring name = object[U("name")].as_string();
			}
			});

		is_connected_ = true;
		return true;
	}

	void WebSocket::Close() {
		client_.close().wait();
	}
}  // namespace monitor_client