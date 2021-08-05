#ifndef MONITOR_CLIENT_EVENT_FILTER_DUMMY_H_
#define MONITOR_CLIENT_EVENT_FILTER_DUMMY_H_

#include <string>

#include "common_struct.h"
#include "base_event_filter.h"
#include "event_queue.h"

namespace monitor_client {
	class EventFilterDummy : public BaseEventFilter {
	public:
		EventFilterDummy() = default;

		EventFilterDummy(const EventFilterDummy&) = default;
		EventFilterDummy& operator=(const EventFilterDummy&) = default;

		EventFilterDummy(EventFilterDummy&&) = default;
		EventFilterDummy& operator=(EventFilterDummy&&) = default;

		~EventFilterDummy() override = default;

		void UploadFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ItemInfo& item_info) const override;
		void DownloadFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
		void RenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const override;
		void RemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
		void LocalRenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const override;
		void LocalRemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
		void ConflictFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
	};
}  // namespace monitor_client
#endif