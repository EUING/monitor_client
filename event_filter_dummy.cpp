#include "event_filter_dummy.h"

#include <string>

#include "upload_event.h"
#include "download_event.h"
#include "rename_event.h"
#include "remove_event.h"
#include "conflict_event.h"
#include "local_rename_event.h"
#include "local_remove_event.h"
#include "common_utility.h"

namespace monitor_client {
	void EventFilterDummy::UploadFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ItemInfo& item_info) const {
		event_queue->Push(std::make_unique<UploadEvent>(item_info));
	}

	void EventFilterDummy::DownloadFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		event_queue->Push(std::make_unique<DownloadEvent>(relative_path));
	}

	void EventFilterDummy::RenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const {
		event_queue->Push(std::make_unique<RenameEvent>(change_name_info));
	}

	void EventFilterDummy::RemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		event_queue->Push(std::make_unique<RemoveEvent>(relative_path));
	}

	void EventFilterDummy::LocalRenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const {
		event_queue->Push(std::make_unique<LocalRenameEvent>(change_name_info));
	}

	void EventFilterDummy::LocalRemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		event_queue->Push(std::make_unique<LocalRemoveEvent>(relative_path));
	}

	void EventFilterDummy::ConflictFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		event_queue->Push(std::make_unique<ConflictEvent>(relative_path));
	}
}  // namespace monitor_client