#ifndef REST_CLIENT_CHANGE_INFO_QUEUE_H_
#define REST_CLIENT_CHANGE_INFO_QUEUE_H_

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

#include "common_utility.h"

namespace my_rest_client {
	class ChangeInfoQueue {
	public:
		ChangeInfoQueue();

		ChangeInfoQueue(const ChangeInfoQueue&) = delete;
		ChangeInfoQueue& operator=(const ChangeInfoQueue&) = delete;

		ChangeInfoQueue(ChangeInfoQueue&&) = delete;
		ChangeInfoQueue& operator=(ChangeInfoQueue&&) = delete;

		~ChangeInfoQueue();

		void Push(const common_utility::ChangeInfo& change_info);
		std::optional<common_utility::ChangeInfo> Pop();
		void Break();

	private:
		std::queue<common_utility::ChangeInfo> change_info_;
		std::mutex change_info_m_;
		std::condition_variable change_info_cv_;
		bool break_;  // Pop 함수 종료를 위한 변수
	};
}  // namespace my_rest_client
#endif