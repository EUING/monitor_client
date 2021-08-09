#ifndef MONITOR_CLIENT_INITIAL_DIFF_EVENT_PUSHER_H_
#define MONITOR_CLIENT_INITIAL_DIFF_EVENT_PUSHER_H_

#include <memory>

#include "base_event_filter.h"
#include "event_pusher.h"
#include "event_queue.h"
#include "diff_check.h"

namespace monitor_client {
	class InitialDiffEventPusher : public EventPusher {
	public:
		InitialDiffEventPusher(const diff_check::ServerDiffList& server_diff_list) : EventPusher(), server_diff_list_(server_diff_list) {}

		InitialDiffEventPusher(const InitialDiffEventPusher&) = default;
		InitialDiffEventPusher& operator=(const InitialDiffEventPusher&) = default;

		InitialDiffEventPusher(InitialDiffEventPusher&&) = default;
		InitialDiffEventPusher& operator=(InitialDiffEventPusher&&) = default;

		~InitialDiffEventPusher() override = default;

		void PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const override;

	private:
		diff_check::ServerDiffList server_diff_list_;
	};
}  // namespace monitor_client
#endif