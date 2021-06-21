#ifndef REST_CLIENT_CHANGE_INFO_QUEUE_H_
#define REST_CLIENT_CHANGE_INFO_QUEUE_H_

#include <Windows.h>

#include <optional>
#include <mutex>
#include <queue>
#include <string>

namespace my_rest_client {
	struct ChangeInfo {
		DWORD action;
		std::wstring full_path;

	};

	class ChangeInfoQueue {
	public:
		ChangeInfoQueue();

		ChangeInfoQueue(const ChangeInfoQueue&) = delete;
		ChangeInfoQueue& operator=(const ChangeInfoQueue&) = delete;

		ChangeInfoQueue(ChangeInfoQueue&&) = delete;
		ChangeInfoQueue& operator=(ChangeInfoQueue&&) = delete;

		~ChangeInfoQueue();

		void Push(const ChangeInfo& change_info);
		std::optional<ChangeInfo> Pop();
		void Finish();

	private:
		std::queue<ChangeInfo> change_info_;
		std::mutex change_info_m_;
		std::condition_variable change_info_cv_;
		bool is_finish;  // Pop 함수 종료를 위한 변수
	};
}  // namespace my_rest_client
#endif