#ifndef MONITOR_CLIENT_EVENT_CONSUMER_H_
#define MONITOR_CLIENT_EVENT_CONSUMER_H_

#include <memory>
#include <future>

#include "event_queue.h"
#include "item_request.h"
#include "item_s3.h"
#include "local_db.h"

namespace monitor_client {
	class EventConsumer {
	public:
		EventConsumer(const std::shared_ptr<EventQueue>& event_queue, const std::shared_ptr<ItemHttp>& item_http, const std::shared_ptr<ItemS3>& item_s3, const std::shared_ptr<LocalDb>& local_db);

		EventConsumer(const EventConsumer&) = delete;
		EventConsumer& operator=(const EventConsumer&) = delete;

		EventConsumer(EventConsumer&&) = delete;
		EventConsumer& operator=(EventConsumer&&) = delete;

		~EventConsumer();

		bool Run();
		void Stop();
		bool IsRunning() const;

	private:
		std::future<void> thread_future_;
		std::shared_ptr<EventQueue> event_queue_;
		ItemRequest item_request_;
	};
}  // namespace monitor_client
#endif