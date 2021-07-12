#include "event_queue.h"

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "common_utility.h"

namespace monitor_client {
	EventQueue::EventQueue() : event_queue_(), notify_queue_m_(), notify_queue_cv_(), break_(false) {
	}

	EventQueue::~EventQueue() {
		Break();
	}

	void EventQueue::Push(std::unique_ptr<const monitor_client::BaseEvent>&& event) {
		std::unique_lock lock(notify_queue_m_);
		event_queue_.push(std::move(event));

		notify_queue_cv_.notify_one();
	}

	void EventQueue::Pop() {
		std::unique_lock lock(notify_queue_m_);
		event_queue_.pop();
	}

	const std::unique_ptr<const monitor_client::BaseEvent>& EventQueue::Front() {
		std::unique_lock lock(notify_queue_m_);

		notify_queue_cv_.wait(lock, [this]() { return (!event_queue_.empty() || break_); });
		if (break_) {
			break_ = false;
			return nullptr;
		}

		return event_queue_.front();
	}

	void EventQueue::Break() {
		break_ = true;
		notify_queue_cv_.notify_all();
	}
}  // namespace monitor_client