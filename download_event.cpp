#include "download_event.h"

#include "item_request.h"

namespace monitor_client {
	bool DownloadEvent::Execute(ItemRequest& item_request) const {
		return item_request.DownloadRequest(relative_path_);
	}
}  // namespace monitor_client