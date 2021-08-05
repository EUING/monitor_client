#include "broadcast_event_pusher.h"

#include <memory>

#include <cpprest/json.h>

#include "base_event_filter.h"
#include "event_queue.h"
#include "common_struct.h"

namespace monitor_client {
	void BroadcastEventPusher::PushEvent(std::shared_ptr<BaseEventFilter> event_filter, std::shared_ptr<EventQueue> event_queue) const {
		web::json::value json_object = web::json::value::parse(json_);
		web::json::object object = json_object.as_object();

		std::wstring event = object[U("event")].as_string();
		if (L"download" == event) {
			std::wstring name = object[U("name")].as_string();
			event_filter->DownloadFilter(event_queue, name);
		}
		else if (L"rename" == event) {
			std::wstring old_name = object[U("old_name")].as_string();
			std::wstring new_name = object[U("new_name")].as_string();

			common_utility::ChangeNameInfo change_name_info{ old_name, new_name };
			event_filter->LocalRenameFilter(event_queue, change_name_info);
		}
		else if (L"remove" == event) {
			std::wstring name = object[U("name")].as_string();
			event_filter->LocalRemoveFilter(event_queue, name);
		}
	}
}  // namespace monitor_client