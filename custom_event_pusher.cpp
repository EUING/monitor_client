#include "custom_event_pusher.h"

#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "event_queue.h"
#include "common_struct.h"
#include "common_utility.h"

namespace monitor_client {
	void CustomEventPusher::PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const {
		for (const auto& iter : server_diff_list_.upload_request_list) {
			if (!common_utility::HasIgnore(iter.name)) {
				event_filter->UploadFilter(event_queue, iter);
			}
		}

		for (const auto& iter : server_diff_list_.download_request_list) {
			if (!common_utility::HasIgnore(iter)) {
				event_filter->DownloadFilter(event_queue, iter);
			}
		}

		for (const auto& iter : server_diff_list_.remove_list) {
			if (!common_utility::HasIgnore(iter)) {
				event_filter->LocalRemoveFilter(event_queue, iter);
			}
		}

		for (const auto& iter : server_diff_list_.conflict_list) {
			if (!common_utility::HasIgnore(iter)) {
				event_filter->ConflictFilter(event_queue, iter);
			}
		}
	}
}  // namespace monitor_client