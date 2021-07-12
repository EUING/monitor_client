#include "event_consumer.h"

#include <iostream>
#include <chrono>
#include <future>

#include "event_queue.h"
#include "item_request.h"

namespace monitor_client {
	EventConsumer::EventConsumer(const std::shared_ptr<EventQueue>& event_queue, const common_utility::NetworkInfo& network_info, std::unique_ptr<ItemDao>&& item_dao) :
		thread_future_{}, 
		event_queue_(event_queue),
		item_request_(network_info, std::move(item_dao)) {
	}

	EventConsumer::~EventConsumer() {
		Stop();
	}

	bool EventConsumer::Run() {
		if (IsRunning())
			return false;

		thread_future_ = std::async(std::launch::async, [this]() {
			if (event_queue_) {
				while (true) {
					if (!event_queue_->Front()) {
						return;
					}

					if (event_queue_->Front()->Execute(item_request_)) {
						event_queue_->Pop();
					}
					else {
						std::wcerr << L"ItemManager::Run: Execute Fail: " << std::endl;
						std::this_thread::sleep_for(std::chrono::seconds(3));
					}
				}
			}
		});

		if (std::future_status::timeout != thread_future_.wait_for(std::chrono::milliseconds(100))) {
			// thread가 준비 완료될 때까지 대기
			return false;
		}

		return true;
	}

	void EventConsumer::Stop() {
		if (IsRunning()) {
			event_queue_->Break();
			thread_future_.wait();
		}
	}

	bool EventConsumer::IsRunning() const {
		return thread_future_.valid() && std::future_status::timeout == thread_future_.wait_for(std::chrono::milliseconds(0));
	}
}  // namespace monitor_client