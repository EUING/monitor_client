#ifndef REST_CLIENT_NOTIFY_QUEUE_H_
#define REST_CLIENT_NOTIFY_QUEUE_H_

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "common_utility.h"

namespace my_rest_client {
	class NotifyQueue {
	public:
		NotifyQueue();

		NotifyQueue(const NotifyQueue&) = delete;
		NotifyQueue& operator=(const NotifyQueue&) = delete;

		NotifyQueue(NotifyQueue&&) = delete;
		NotifyQueue& operator=(NotifyQueue&&) = delete;

		~NotifyQueue();

		void Push(const common_utility::ChangeObjectInfo& change_info);
		std::optional<common_utility::ChangeObjectInfo> Pop();
		void Break();

	private:
		std::queue<common_utility::ChangeObjectInfo> change_info_;
		std::mutex notify_queue_m_;
		std::condition_variable notify_queue_cv_;
		bool break_;  // Pop �Լ� ���Ḧ ���� ����
	};
}  // namespace my_rest_client
#endif