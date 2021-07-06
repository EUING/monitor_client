#ifndef MONITOR_CLIENT_EVENT_HANDLER_H_
#define MONITOR_CLIENT_EVENT_HANDLER_H_

#include <Windows.h>
#include <stdint.h>

#include <string>

#include "notify_queue.h"

namespace monitor_client {
	class EventHandler {
	public:
		explicit EventHandler(std::shared_ptr<NotifyQueue> notify_queue);

		EventHandler(const EventHandler&) = default;
		EventHandler& operator=(const EventHandler&) = default;

		EventHandler(EventHandler&&) = default;
		EventHandler& operator=(EventHandler&&) = default;

		~EventHandler() = default;

		void PushEvent(const uint8_t* buffer);

	private:
		void PushAddEvent(const std::wstring& relative_path);

		std::shared_ptr<NotifyQueue> notify_queue_;
	};

}  // namespace monitor_client
#endif