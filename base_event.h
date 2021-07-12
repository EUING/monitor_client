#ifndef MONITOR_CLIENT_BASE_EVENT_H_
#define MONITOR_CLIENT_BASE_EVENT_H_

#include "item_request.h"

namespace monitor_client {
	class BaseEvent {
	public:
		BaseEvent() = default;

		BaseEvent(const BaseEvent&) = default;
		BaseEvent& operator=(const BaseEvent&) = default;

		BaseEvent(BaseEvent&&) = default;
		BaseEvent& operator=(BaseEvent&&) = default;

		virtual ~BaseEvent() = default;

		virtual bool Execute(ItemRequest& item_request) const = 0;
	};
}  // namespace monitor_client
#endif