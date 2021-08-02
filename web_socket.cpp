#include "web_socket.h"

#include <iostream>
#include <string>

#include <cpprest/uri_builder.h>

#include "common_struct.h"

namespace monitor_client {
	WebSocket::WebSocket(const common_utility::NetworkInfo& info) : builder_(), client_(), is_connected_(false) {
		builder_.set_scheme(U("ws")).set_host(info.host).set_port(info.port);
	}

	WebSocket::~WebSocket() {
		client_.close().wait();
	}

	bool WebSocket::Connect() {
		Concurrency::task_status status = client_.connect(builder_.to_uri()).wait();
		if (Concurrency::task_status::completed != status) {
			std::wcerr << L"WebSocket::Connect: client_.connect fail: " << builder_.to_string() << std::endl;
			return false;
		}

		client_.set_message_handler([](web::websockets::client::websocket_incoming_message msg) {
			Concurrency::task<std::string> message = msg.extract_string();
			auto str = message.get();
			std::cout << str << std::endl;
			});

		web::websockets::client::websocket_outgoing_message msg;
		msg.set_utf8_message("hello");
		client_.send(msg).wait();

		is_connected_ = true;
		return true;
	}

	void WebSocket::Close() {
		client_.close().wait();
	}
}  // namespace monitor_client