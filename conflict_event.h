#ifndef MONITOR_CLIENT_CONFLICT_EVENT_H_
#define MONITOR_CLIENT_CONFLICT_EVENT_H_

#include <string>

#include "base_event.h"
#include "item_request.h"

namespace monitor_client {
	class ConflictEvent : public BaseEvent {
	public:
		ConflictEvent(const std::wstring& relative_path) : BaseEvent(), relative_path_(relative_path) {}

		ConflictEvent(const ConflictEvent&) = default;
		ConflictEvent& operator=(const ConflictEvent&) = default;

		ConflictEvent(ConflictEvent&&) = default;
		ConflictEvent& operator=(ConflictEvent&&) = default;

		~ConflictEvent() override = default;

		bool operator()(ItemRequest& item_request) const override;

	private:
		std::wstring relative_path_;
	};
}  // namespace monitor_client
#endif