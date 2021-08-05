#ifndef MONITOR_CLIENT_BASE_EVENT_FILTER_H_
#define MONITOR_CLIENT_BASE_EVENT_FILTER_H_

#include <memory>
#include <string>

#include "common_struct.h"
#include "event_queue.h"

namespace monitor_client {
	class BaseEventFilter {
	public:
		BaseEventFilter() = default;

		BaseEventFilter(const BaseEventFilter&) = default;
		BaseEventFilter& operator=(const BaseEventFilter&) = default;

		BaseEventFilter(BaseEventFilter&&) = default;
		BaseEventFilter& operator=(BaseEventFilter&&) = default;

		virtual ~BaseEventFilter() = default;

		virtual void UploadFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ItemInfo& item_info) const = 0;
		virtual void DownloadFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const = 0;
		virtual void RenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& item_info) const = 0;
		virtual void RemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const = 0;
		virtual void LocalRenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const = 0;
		virtual void LocalRemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const = 0;
		virtual void ConflictFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const = 0;
	};
}  // namespace monitor_client
#endif