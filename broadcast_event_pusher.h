#ifndef MONITOR_CLIENT_BROADCAST_EVENT_PUSHER_H_
#define MONITOR_CLIENT_BROADCAST_EVENT_PUSHER_H_

#include <string>
#include <memory>

#include "base_event_filter.h"
#include "event_pusher.h"
#include "event_queue.h"

namespace monitor_client {
	class BroadcastEventPusher : public EventPusher {
	public:
		BroadcastEventPusher(const std::string& json) : EventPusher(), json_(json) {}

		BroadcastEventPusher(const BroadcastEventPusher&) = default;
		BroadcastEventPusher& operator=(const BroadcastEventPusher&) = default;

		BroadcastEventPusher(BroadcastEventPusher&&) = default;
		BroadcastEventPusher& operator=(BroadcastEventPusher&&) = default;

		~BroadcastEventPusher() override = default;

		void PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const override;
	private:
		std::string json_;
	};
}  // namespace monitor_client
#endif