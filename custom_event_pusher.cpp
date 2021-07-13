#include "custom_event_pusher.h"

#include <vector>
#include <memory>
#include <string>

#include "event_queue.h"
#include "common_struct.h"
#include "upload_event.h"
#include "download_event.h"
#include "conflict_event.h"
#include "local_remove_event.h"

namespace monitor_client {
	void CustomEventPusher::PushEvent(std::shared_ptr<EventQueue> event_queue) const {
		for (const auto& iter : server_diff_list_.upload_request_list) {
			event_queue->Push(std::make_unique<UploadEvent>(iter));
		}

		for (const auto& iter : server_diff_list_.download_request_list) {
			event_queue->Push(std::make_unique<DownloadEvent>(iter));
		}

		for (const auto& iter : server_diff_list_.remove_list) {
			event_queue->Push(std::make_unique<LocalRemoveEvent>(iter));
		}

		for (const auto& iter : server_diff_list_.conflict_list) {
			event_queue->Push(std::make_unique<ConflictEvent>(iter));
		}
	}
}  // namespace monitor_client