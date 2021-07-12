#ifndef MONITOR_CLIENT_RENAME_EVENT_H_
#define MONITOR_CLIENT_RENAME_EVENT_H_

#include <string>

#include "base_event.h"
#include "common_struct.h"
#include "item_request.h"

namespace monitor_client {
	class RenameEvent : public BaseEvent {
	public:
		RenameEvent(const common_utility::ChangeNameInfo& change_name_info) : BaseEvent(), change_name_info_(change_name_info) {}

		RenameEvent(const RenameEvent&) = default;
		RenameEvent& operator=(const RenameEvent&) = default;

		RenameEvent(RenameEvent&&) = default;
		RenameEvent& operator=(RenameEvent&&) = default;

		~RenameEvent() override = default;

		bool operator()(ItemRequest& item_request) const override;

	private:
		common_utility::ChangeNameInfo change_name_info_;
	};
}  // namespace monitor_client
#endif