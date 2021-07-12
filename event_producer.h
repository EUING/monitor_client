#ifndef MONITOR_CLIENT_EVENT_PRODUCER_H_
#define MONITOR_CLIENT_EVENT_PRODUCER_H_

#include <stdint.h>

#include <memory>
#include <string>

#include "event_queue.h"
#include "diff_check.h"

namespace monitor_client {
	class EventProducer {
	public:
		explicit EventProducer(const std::shared_ptr<EventQueue>& event_queue);

		EventProducer(const EventProducer&) = default;
		EventProducer& operator=(const EventProducer&) = default;

		EventProducer(EventProducer&&) = default;
		EventProducer& operator=(EventProducer&&) = default;

		~EventProducer() = default;

		void PushEvent(std::shared_ptr<const uint8_t[]> buffer);
		void PushEvent(diff_check::ServerDiffList diff_list);

	private:
		void PushAddEvent(const std::wstring& relative_path);

		std::shared_ptr<EventQueue> event_queue_;
	};
}  // namespace monitor_client
#endif