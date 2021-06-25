#ifndef MONITOR_CLIENT_NOTIFY_QUEUE_H_
#define MONITOR_CLIENT_NOTIFY_QUEUE_H_

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "common_utility.h"

namespace monitor_client {
	class NotifyQueue {
	public:
		NotifyQueue();

		NotifyQueue(const NotifyQueue&) = delete;
		NotifyQueue& operator=(const NotifyQueue&) = delete;

		NotifyQueue(NotifyQueue&&) = delete;
		NotifyQueue& operator=(NotifyQueue&&) = delete;

		~NotifyQueue();

		void Push(const common_utility::ChangeItemInfo& change_info);
		std::optional<common_utility::ChangeItemInfo> Pop();
		void Break();

	private:
		std::queue<common_utility::ChangeItemInfo> change_info_;
		std::mutex notify_queue_m_;
		std::condition_variable notify_queue_cv_;
		bool break_;  // Pop 함수 종료를 위한 변수
	};
}  // namespace monitor_client
#endif