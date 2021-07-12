#include "rename_event.h"

#include <iostream>

#include "item_request.h"

namespace monitor_client {
	bool RenameEvent::operator()(ItemRequest& item_request) const {
		return item_request.RenameRequest(change_name_info_);
	}
}  // namespace monitor_client