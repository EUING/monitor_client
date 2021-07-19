#include "custom_event_pusher.h"

#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "event_queue.h"
#include "common_struct.h"
#include "upload_event.h"
#include "download_event.h"
#include "conflict_event.h"
#include "local_remove_event.h"

namespace monitor_client {
	void CustomEventPusher::PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const {
		for (const auto& iter : server_diff_list_.upload_request_list) {
			event_filter->UploadFilter(event_queue, iter);
		}

		for (const auto& iter : server_diff_list_.download_request_list) {
			event_filter->DownloadFilter(event_queue, iter);
		}

		for (const auto& iter : server_diff_list_.remove_list) {
			event_filter->LocalRemoveFilter(event_queue, iter);
		}

		for (const auto& iter : server_diff_list_.conflict_list) {
			event_filter->ConflictFilter(event_queue, iter);
		}
	}
}  // namespace monitor_client