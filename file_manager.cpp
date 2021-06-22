#include "file_manager.h"

#include <Windows.h>

#include <future>
#include <optional>
#include <sstream>
#include <iomanip>

#include "common_utility.h"
#include "change_info_queue.h"

namespace my_rest_client {
	FileManager::FileManager(ChangeInfoQueue* change_info) : thread_future_{}, change_info_(change_info) {
	}

	FileManager::~FileManager() {
		Stop();
	}

	bool FileManager::Run() {
		if (IsRunning())
			return false;

		thread_future_ = std::async(std::launch::async, [this]() {
			if (change_info_) {
				while (true) {				
					std::optional<common_utility::ChangeInfo> pop = change_info_->Pop();
					if (!pop.has_value())
						return;

					common_utility::ChangeInfo change_info = pop.value();
				}
			}
		});

		if (std::future_status::timeout != thread_future_.wait_for(std::chrono::milliseconds(100))) {
			// thread가 준비 완료될 때까지 대기
			return false;
		}

		return true;
	}

	void FileManager::Stop() {
		if (IsRunning()) {
			change_info_->Break();
			thread_future_.wait();
		}
	}

	bool FileManager::IsRunning() {
		return thread_future_.valid() && std::future_status::timeout == thread_future_.wait_for(std::chrono::milliseconds(0));
	}
}  // namespace my_rest_client