#include "notify_queue.h"

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "common_utility.h"

namespace monitor_client {
	NotifyQueue::NotifyQueue(): change_info_(), notify_queue_m_(), notify_queue_cv_(), break_(false) {
	}

	NotifyQueue::~NotifyQueue() {
		Break();
	}

	void NotifyQueue::Push(const common_utility::ChangeItemInfo& change_info) {
		std::unique_lock lock(notify_queue_m_);
		change_info_.push(change_info);

		notify_queue_cv_.notify_one();
	}

	void NotifyQueue::Pop() {
		std::unique_lock lock(notify_queue_m_);
		change_info_.pop();
	}

	std::optional<common_utility::ChangeItemInfo> NotifyQueue::Front() {
		std::unique_lock lock(notify_queue_m_);

		notify_queue_cv_.wait(lock, [this]() { return (!change_info_.empty() || break_); });
		if (break_) {
			break_ = false;
			return std::nullopt;
		}

		auto front = change_info_.front();

		return front;
	}

	void NotifyQueue::Break() {
		break_ = true;
		notify_queue_cv_.notify_all();
	}
}  // namespace monitor_client