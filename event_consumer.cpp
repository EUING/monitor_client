#include "event_consumer.h"

#include <iostream>
#include <chrono>
#include <future>
#include <memory>

#include "event_queue.h"
#include "item_request.h"
#include "item_s3.h"
#include "local_db.h"

namespace monitor_client {
	EventConsumer::EventConsumer(const std::shared_ptr<EventQueue>& event_queue, const std::shared_ptr<ItemHttp>& item_http, const std::shared_ptr<ItemS3>& item_s3, const std::shared_ptr<LocalDb>& local_db) :
		thread_future_{},
		event_queue_(event_queue),
		item_request_(item_http, item_s3, local_db) {
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