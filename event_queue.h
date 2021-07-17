#ifndef MONITOR_CLIENT_EVENT_QUEUE_H_
#define MONITOR_CLIENT_EVENT_QUEUE_H_

#include <memory>
#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "base_event.h"

namespace monitor_client {
	class EventQueue {
	public:
		EventQueue();

		EventQueue(const EventQueue&) = delete;
		EventQueue& operator=(const EventQueue&) = delete;

		EventQueue(EventQueue&&) = delete;
		EventQueue& operator=(EventQueue&&) = delete;

		~EventQueue();

		void Push(std::unique_ptr<const monitor_client::BaseEvent>&& event);
		const std::unique_ptr<const monitor_client::BaseEvent>& Front();
		void Pop();
		void Break();
		size_t Size() { return event_queue_.size(); }

	private:
		std::queue<std::unique_ptr<const monitor_client::BaseEvent>> event_queue_;
		std::mutex notify_queue_m_;
		std::condition_variable notify_queue_cv_;
		bool break_;  // Pop 함수 종료를 위한 변수
	};
}  // namespace monitor_client
#endif