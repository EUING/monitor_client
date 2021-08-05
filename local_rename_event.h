#ifndef MONITOR_CLIENT_LOCAL_RENAME_EVENT_H_
#define MONITOR_CLIENT_LOCAL_RENAME_EVENT_H_

#include <string>

#include "base_event.h"
#include "common_struct.h"
#include "item_request.h"

namespace monitor_client {
	class LocalRenameEvent : public BaseEvent {
	public:
		LocalRenameEvent(const common_utility::ChangeNameInfo& change_name_info) : BaseEvent(), change_name_info_(change_name_info) {}

		LocalRenameEvent(const LocalRenameEvent&) = default;
		LocalRenameEvent& operator=(const LocalRenameEvent&) = default;

		LocalRenameEvent(LocalRenameEvent&&) = default;
		LocalRenameEvent& operator=(LocalRenameEvent&&) = default;

		~LocalRenameEvent() override = default;

		bool Execute(ItemRequest& item_request) const override;

	private:
		common_utility::ChangeNameInfo change_name_info_;
	};
}  // namespace monitor_client
#endif