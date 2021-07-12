#ifndef MONITOR_CLIENT_LOCAL_REMOVE_EVENT_H_
#define MONITOR_CLIENT_LOCAL_REMOVE_EVENT_H_

#include <string>

#include "base_event.h"
#include "item_request.h"

namespace monitor_client {
	class LocalRemoveEvent : public BaseEvent {
	public:
		LocalRemoveEvent(const std::wstring& relative_path) : BaseEvent(), relative_path_(relative_path) {}

		LocalRemoveEvent(const LocalRemoveEvent&) = default;
		LocalRemoveEvent& operator=(const LocalRemoveEvent&) = default;

		LocalRemoveEvent(LocalRemoveEvent&&) = default;
		LocalRemoveEvent& operator=(LocalRemoveEvent&&) = default;

		~LocalRemoveEvent() override = default;

		bool operator()(ItemRequest& item_request) const override;

	private:
		std::wstring relative_path_;
	};
}  // namespace monitor_client
#endif