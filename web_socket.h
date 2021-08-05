#ifndef MONITOR_CLIENT_WEB_SOCKET_H_
#define MONITOR_CLIENT_WEB_SOCKET_H_

#include <sdkddkver.h>

#include <cpprest/uri_builder.h>
#include <cpprest/ws_client.h>

#include "common_struct.h"
#include "event_producer.h"

namespace monitor_client {
	class WebSocket {
	public:
		WebSocket(const common_utility::NetworkInfo& info, const EventProducer& event_producer);

		WebSocket(const WebSocket&) = delete;
		WebSocket& operator=(const WebSocket&) = delete;

		WebSocket(WebSocket&&) = default;
		WebSocket& operator=(WebSocket&&) = default;

		~WebSocket();

		bool Connect();
		void Close();

	private:
		EventProducer event_producer_;
		web::uri_builder builder_;
		web::websockets::client::websocket_callback_client client_;
		bool is_connected_;
	};
}  // namespace monitor_client
#endif