#include "change_info_queue.h"

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

namespace my_rest_client {
	ChangeInfoQueue::ChangeInfoQueue(): change_info_(), change_info_m_(), change_info_cv_(), is_finish(false) {
	}

	ChangeInfoQueue::~ChangeInfoQueue() {
		Finish();
	}

	void ChangeInfoQueue::Push(const ChangeInfo& change_info) {
		std::unique_lock lock(change_info_m_);
		change_info_.push(change_info);
		lock.unlock();

		change_info_cv_.notify_one();
	}

	std::optional<ChangeInfo> ChangeInfoQueue::Pop() {
		std::unique_lock lock(change_info_m_);
		
		change_info_cv_.wait(lock, [this]() { return (!change_info_.empty() || is_finish); });
		if (is_finish) {
			is_finish = false;
			return std::nullopt;
		}

		ChangeInfo front = change_info_.front();
		change_info_.pop();

		return front;
	}

	void ChangeInfoQueue::Finish() {
		is_finish = true;
		change_info_cv_.notify_all();
	}
}  // namespace my_rest_client