#ifndef MONITOR_CLIENT_CUSTOM_EVENT_PUSHER_H_
#define MONITOR_CLIENT_CUSTOM_EVENT_PUSHER_H_

#include <memory>

#include "base_event_filter.h"
#include "event_pusher.h"
#include "event_queue.h"
#include "diff_check.h"

namespace monitor_client {
	class CustomEventPusher : public EventPusher {
	public:
		CustomEventPusher(const diff_check::ServerDiffList& server_diff_list) : EventPusher(), server_diff_list_(server_diff_list) {}

		CustomEventPusher(const CustomEventPusher&) = default;
		CustomEventPusher& operator=(const CustomEventPusher&) = default;

		CustomEventPusher(CustomEventPusher&&) = default;
		CustomEventPusher& operator=(CustomEventPusher&&) = default;

		~CustomEventPusher() override = default;

		void PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const override;

	private:
		diff_check::ServerDiffList server_diff_list_;
	};
}  // namespace monitor_client
#endif