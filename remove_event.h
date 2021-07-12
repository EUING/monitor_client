#ifndef MONITOR_CLIENT_REMOVE_EVENT_H_
#define MONITOR_CLIENT_REMOVE_EVENT_H_

#include <string>

#include "base_event.h"
#include "item_request.h"

namespace monitor_client {
	class RemoveEvent : public BaseEvent {
	public:
		RemoveEvent(const std::wstring& relative_path) : BaseEvent(), relative_path_(relative_path) {}

		RemoveEvent(const RemoveEvent&) = default;
		RemoveEvent& operator=(const RemoveEvent&) = default;

		RemoveEvent(RemoveEvent&&) = default;
		RemoveEvent& operator=(RemoveEvent&&) = default;

		~RemoveEvent() override = default;

		bool Execute(ItemRequest& item_request) const override;

	private:
		std::wstring relative_path_;
	};
}  // namespace monitor_client
#endif