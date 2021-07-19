#ifndef MONITOR_CLIENT_EVENT_PUSHER_H_
#define MONITOR_CLIENT_EVENT_PUSHER_H_

#include <memory>

#include "base_event_filter.h"
#include "event_queue.h"

namespace monitor_client {
	class EventPusher {
	public:
		EventPusher() = default;

		EventPusher(const EventPusher&) = default;
		EventPusher& operator=(const EventPusher&) = default;

		EventPusher(EventPusher&&) = default;
		EventPusher& operator=(EventPusher&&) = default;

		virtual ~EventPusher() = default;

		virtual void PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const = 0;
	};
}  // namespace monitor_client
#endif