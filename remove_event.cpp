#include "remove_event.h"

#include <iostream>

#include "item_request.h"

namespace monitor_client {
	bool RemoveEvent::operator()(ItemRequest& item_request) const {
		return item_request.RemoveRequest(relative_path_);
	}
}  // namespace monitor_client