#include "event_producer.h"

#include <memory>

#include "event_queue.h"
#include "event_pusher.h"

namespace monitor_client {
	EventProducer::EventProducer(const std::shared_ptr<BaseEventFilter>& event_filter, const std::shared_ptr<EventQueue>& event_queue) : event_filter_(event_filter), event_queue_(event_queue) {
	}

	void EventProducer::PushEvent(std::unique_ptr<EventPusher> event_pusher) {
		event_pusher->PushEvent(event_filter_, event_queue_);
	}
}  // namespace monitor_client