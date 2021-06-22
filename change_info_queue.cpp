#include "change_info_queue.h"

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "common_utility.h"

namespace my_rest_client {
	ChangeInfoQueue::ChangeInfoQueue(): change_info_(), change_info_m_(), change_info_cv_(), break_(false) {
	}

	ChangeInfoQueue::~ChangeInfoQueue() {
		Break();
	}

	void ChangeInfoQueue::Push(const common_utility::ChangeInfo& change_info) {
		std::unique_lock lock(change_info_m_);
		change_info_.push(change_info);
		lock.unlock();

		change_info_cv_.notify_one();
	}

	std::optional<common_utility::ChangeInfo> ChangeInfoQueue::Pop() {
		std::unique_lock lock(change_info_m_);
		
		change_info_cv_.wait(lock, [this]() { return (!change_info_.empty() || break_); });
		if (break_) {
			break_ = false;
			return std::nullopt;
		}

		common_utility::ChangeInfo front = change_info_.front();
		change_info_.pop();

		return front;
	}

	void ChangeInfoQueue::Break() {
		break_ = true;
		change_info_cv_.notify_all();
	}
}  // namespace my_rest_client