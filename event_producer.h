#ifndef MONITOR_CLIENT_EVENT_PRODUCER_H_
#define MONITOR_CLIENT_EVENT_PRODUCER_H_

#include <memory>

#include "event_queue.h"
#include "event_pusher.h"

namespace monitor_client {
	class EventProducer {
	public:
		explicit EventProducer(const std::shared_ptr<EventQueue>& event_queue);

		EventProducer(const EventProducer&) = default;
		EventProducer& operator=(const EventProducer&) = default;

		EventProducer(EventProducer&&) = default;
		EventProducer& operator=(EventProducer&&) = default;

		~EventProducer() = default;

		void PushEvent(std::unique_ptr<EventPusher> event_pusher);

		std::shared_ptr<EventQueue> event_queue_;
	};
}  // namespace monitor_client
#endif