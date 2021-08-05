#include "remove_event.h"

#include "item_request.h"

namespace monitor_client {
	bool RemoveEvent::Execute(ItemRequest& item_request) const {
		return item_request.RemoveRequest(relative_path_);
	}
}  // namespace monitor_client