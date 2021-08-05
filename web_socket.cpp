#include "web_socket.h"

#include <memory>
#include <iostream>
#include <string>

#include <cpprest/uri_builder.h>

#include "broadcast_event_pusher.h"
#include "event_producer.h"
#include "common_struct.h"

namespace monitor_client {
	WebSocket::WebSocket(const common_utility::NetworkInfo& info, const EventProducer& event_producer) : event_producer_(event_producer), builder_(), client_(), is_connected_(false) {
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

		client_.set_message_handler([this](web::websockets::client::websocket_incoming_message msg) {
			Concurrency::task<std::string> message = msg.extract_string();
			
			event_producer_.PushEvent(std::make_unique<BroadcastEventPusher>(message.get()));
			});

		is_connected_ = true;
		return true;
	}

	void WebSocket::Close() {
		web::websockets::client::websocket_outgoing_message msg;
		msg.set_pong_message();
		client_.send(msg).get();
		client_.close().get();
	}
}  // namespace monitor_client