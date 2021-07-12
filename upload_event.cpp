#include "upload_event.h"

#include "item_request.h"

namespace monitor_client {
	bool UploadEvent::operator()(ItemRequest& item_request) const {
		return item_request.UploadRequest(item_info_);
	}
}  // namespace monitor_client