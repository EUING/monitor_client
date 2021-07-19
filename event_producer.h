#ifndef MONITOR_CLIENT_EVENT_PRODUCER_H_
#define MONITOR_CLIENT_EVENT_PRODUCER_H_

#include <memory>

#include "event_queue.h"
#include "event_pusher.h"
#include "base_event_filter.h"

namespace monitor_client {
	class EventProducer {
	public:
		EventProducer(const std::shared_ptr<BaseEventFilter>& event_filter, const std::shared_ptr<EventQueue>& event_queue);

		EventProducer(const EventProducer&) = default;
		EventProducer& operator=(const EventProducer&) = default;

		EventProducer(EventProducer&&) = default;
		EventProducer& operator=(EventProducer&&) = default;

		~EventProducer() = default;

		void PushEvent(std::unique_ptr<EventPusher> event_pusher);
		void SetEventFilter(const std::shared_ptr<BaseEventFilter>& event_filter) { event_filter_ = event_filter; }
		std::shared_ptr<BaseEventFilter> GetEventFilter() { return event_filter_; }

		std::shared_ptr<BaseEventFilter> event_filter_;
		std::shared_ptr<EventQueue> event_queue_;
	};
}  // namespace monitor_client
#endif