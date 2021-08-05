#ifndef MONITOR_CLIENT_EVENT_FILTER_H_
#define MONITOR_CLIENT_EVENT_FILTER_H_

#include <memory>
#include <string>

#include "common_struct.h"
#include "base_event_filter.h"
#include "event_queue.h"
#include "local_db.h"

namespace monitor_client {
	class EventFilter : public BaseEventFilter {
	public:
		EventFilter(const std::shared_ptr<LocalDb>& local_db) : local_db_(local_db), BaseEventFilter() {};

		EventFilter(const EventFilter&) = default;
		EventFilter& operator=(const EventFilter&) = default;

		EventFilter(EventFilter&&) = default;
		EventFilter& operator=(EventFilter&&) = default;

		~EventFilter() override = default;

		void UploadFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ItemInfo& item_info) const override;
		void DownloadFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
		void RenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const override;
		void RemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
		void LocalRenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const override;
		void LocalRemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;
		void ConflictFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const override;

	private:
		std::shared_ptr<LocalDb> local_db_;
	};
}  // namespace monitor_client
#endif