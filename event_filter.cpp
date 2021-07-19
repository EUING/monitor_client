#include "event_filter.h"

#include <optional>
#include <string>

#include "upload_event.h"
#include "download_event.h"
#include "rename_event.h"
#include "remove_event.h"
#include "conflict_event.h"
#include "local_remove_event.h"
#include "common_utility.h"

namespace {
	bool HasIgnore(const std::wstring& relative_path) {
		return std::wstring::npos != relative_path.find(L".ignore");
	}

	bool HasConflict(const std::wstring& relative_path) {
		return std::wstring::npos != relative_path.find(L".conflict");
	}
}  // namespace

namespace monitor_client {
	void EventFilter::UploadFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ItemInfo& item_info) const {
		if (HasIgnore(item_info.name) || HasConflict(item_info.name)) {
			return;
		}

		std::optional<common_utility::ItemInfo> item_info_opt = local_db_->GetItemInfo(item_info.name);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"BasicEventFilter::UploadFilter: local_db_.GetItemInfo Fail: " << item_info.name << std::endl;
			return;
		}

		auto local_item_info = item_info_opt.value();
		if (!local_item_info.name.empty() && local_item_info.hash == item_info.hash) {
			return;
		}

		std::wclog << L"EventFilter UploadEvent: " << item_info.name << std::endl;
		event_queue->Push(std::make_unique<UploadEvent>(item_info));
	}

	void EventFilter::DownloadFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (HasIgnore(relative_path) || HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter DownloadEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<DownloadEvent>(relative_path));
	}

	void EventFilter::RenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const {
		if (HasIgnore(change_name_info.old_name)) {
			return;
		}

		bool has_old = HasConflict(change_name_info.old_name);
		bool has_new = HasConflict(change_name_info.new_name);

		std::unique_ptr<BaseEvent> event;
		if (has_old && has_new) {
			return;  // 둘다 있는 경우 Event 무시
		}
		else if (has_new) {
			return;  // 충돌로 인해 생긴 Event이므로 무시
		}
		else if (has_old) {
			std::optional<common_utility::ItemInfo> item_info_opt = common_utility::GetItemInfo(change_name_info.new_name);
			if (!item_info_opt.has_value()) {
				std::wcerr << L"BasicEventFilter::RenameFilter: local_db_.GetItemInfo Fail: " << change_name_info.new_name << std::endl;
				return;
			}

			auto item_info = item_info_opt.value();
			std::wclog << L"EventFilter UploadEvent: " << item_info.name << std::endl;
			event = std::make_unique<UploadEvent>(item_info);  // 사용자가 충돌을 해결하여 파일명을 수정한 경우
		}
		else {
			std::wclog << L"EventFilter RenameEvent: " << change_name_info.old_name << std::endl;
			event = std::make_unique<RenameEvent>(change_name_info);  // 충돌이 없는 파일에 대한 파일명 수정
		}

		event_queue->Push(std::move(event));
	}

	void EventFilter::RemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (HasIgnore(relative_path) || HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter RemoveEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<RemoveEvent>(relative_path));
	}

	void EventFilter::LocalRemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (HasIgnore(relative_path) || HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter LocalRemoveEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<LocalRemoveEvent>(relative_path));
	}

	void EventFilter::ConflictFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (HasIgnore(relative_path) || HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter ConflictEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<ConflictEvent>(relative_path));
	}
}  // namespace monitor_client