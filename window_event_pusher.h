#ifndef MONITOR_CLIENT_WINDOW_EVENT_PUSHER_H_
#define MONITOR_CLIENT_WINDOW_EVENT_PUSHER_H_

#include <stdint.h>

#include <memory>

#include "event_pusher.h"
#include "event_queue.h"

namespace monitor_client {
	class WindowEventPusher : public EventPusher {
	public:
		WindowEventPusher(const std::shared_ptr<uint8_t[]>& buffer) : EventPusher(), buffer_(buffer) {}

		WindowEventPusher(const WindowEventPusher&) = default;
		WindowEventPusher& operator=(const WindowEventPusher&) = default;

		WindowEventPusher(WindowEventPusher&&) = default;
		WindowEventPusher& operator=(WindowEventPusher&&) = default;

		~WindowEventPusher() override = default;

		void PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const override;

	private:
		void PushAddEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const;

		std::shared_ptr<uint8_t[]> buffer_;
	};
}  // namespace monitor_client
#endif