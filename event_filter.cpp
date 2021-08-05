#include "event_filter.h"

#include <optional>
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
	void EventFilter::UploadFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ItemInfo& item_info) const {
		if (common_utility::HasConflict(item_info.name)) {
			return;
		}

		std::optional<common_utility::ItemInfo> item_info_opt = local_db_->GetItemInfo(item_info.name);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"EventFilter::UploadFilter: local_db_.GetItemInfo Fail: " << item_info.name << std::endl;
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
		if (common_utility::HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter DownloadEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<DownloadEvent>(relative_path));
	}

	void EventFilter::RenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const {
		std::unique_ptr<BaseEvent> event;
		bool ignore_old = common_utility::HasIgnore(change_name_info.old_name);
		bool ignore_new = common_utility::HasIgnore(change_name_info.new_name);

		bool conflict_old = common_utility::HasConflict(change_name_info.old_name);
		bool conflict_new = common_utility::HasConflict(change_name_info.new_name);

		if (!ignore_old && !ignore_new && !conflict_old && !conflict_new) {
			std::optional<common_utility::ItemInfo> item_info_opt = local_db_->GetItemInfo(change_name_info.old_name);
			if (!item_info_opt.has_value()) {
				std::wcerr << L"EventFilter::UploadFilter: local_db_.GetItemInfo Fail: " << change_name_info.old_name << std::endl;
				return;
			}

			auto old_item_info = item_info_opt.value();

			item_info_opt = local_db_->GetItemInfo(change_name_info.new_name);
			if (!item_info_opt.has_value()) {
				std::wcerr << L"EventFilter::UploadFilter: local_db_.GetItemInfo Fail: " << change_name_info.new_name << std::endl;
				return;
			}

			auto new_item_info = item_info_opt.value();

			if (old_item_info.name.empty() && !new_item_info.name.empty()) {
				return;  // �̹� �ݿ��� �̺�Ʈ�� ��� ����
			}

			std::wclog << L"EventFilter RenameEvent: " << change_name_info.old_name << std::endl;
			event = std::make_unique<RenameEvent>(change_name_info);  // �������� ���Ͽ� ���� ���ϸ� ����
		}
		else {
			if (ignore_old && ignore_new) {
				return;  // �Ѵ� �ִ� ��� Event ����
			}
			else if (ignore_old && !ignore_new) {
				std::optional<common_utility::ItemInfo> item_info_opt = common_utility::GetItemInfo(change_name_info.new_name);
				if (!item_info_opt.has_value()) {
					std::wcerr << L"EventFilter::RenameFilter: local_db_.GetItemInfo Fail: " << change_name_info.new_name << std::endl;
					return;
				}

				auto item_info = item_info_opt.value();
				std::wclog << L"EventFilter UploadEvent: " << item_info.name << std::endl;
				event = std::make_unique<UploadEvent>(item_info);  // ����ڰ� .ignore�� ������ ���
			}
			else if (!ignore_old && ignore_new) {
				event = std::make_unique<RemoveEvent>(change_name_info.old_name);  // ����ڰ� ���� Ȥ�� ������ .ignore�� �����Ͽ� ������ ���
			}
			else if (!ignore_old && !ignore_new) {
				if (conflict_old && conflict_new) {
					return;  // �Ѵ� �ִ� ��� Event ����
				}
				else if (conflict_old && !conflict_new) {
					std::optional<common_utility::ItemInfo> item_info_opt = common_utility::GetItemInfo(change_name_info.new_name);
					if (!item_info_opt.has_value()) {
						std::wcerr << L"EventFilter::RenameFilter: local_db_.GetItemInfo Fail: " << change_name_info.new_name << std::endl;
						return;
					}

					auto item_info = item_info_opt.value();
					std::wclog << L"EventFilter UploadEvent: " << item_info.name << std::endl;
					event = std::make_unique<UploadEvent>(item_info);  // ����ڰ� �浹�� �ذ��Ͽ� ���ϸ��� ������ ���
				}
				else if (!conflict_old && conflict_new) {
					return;  // �浹�� ���� ���� Event�̹Ƿ� ����
				}
			}
		}		

		event_queue->Push(std::move(event));
	}

	void EventFilter::RemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (common_utility::HasConflict(relative_path)) {
			return;
		}

		std::optional<common_utility::ItemInfo> item_info_opt = local_db_->GetItemInfo(relative_path);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"EventFilter::RemoveFilter: local_db_.GetItemInfo Fail: " << relative_path << std::endl;
			return;
		}

		if (item_info_opt.value().name.empty()) {
			return;  // �̹� �ݿ��� �̺�Ʈ�� ��� ����
		}

		std::wclog << L"EventFilter RemoveEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<RemoveEvent>(relative_path));
	}

	void EventFilter::LocalRenameFilter(std::shared_ptr<EventQueue> event_queue, const common_utility::ChangeNameInfo& change_name_info) const {
		if (common_utility::HasConflict(change_name_info.old_name)) {
			return;
		}

		if (common_utility::HasConflict(change_name_info.new_name)) {
			return;
		}

		std::wclog << L"EventFilter LocalRenameFilter: " << change_name_info.new_name << std::endl;
		event_queue->Push(std::make_unique<LocalRenameEvent>(change_name_info));
	}

	void EventFilter::LocalRemoveFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (common_utility::HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter LocalRemoveEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<LocalRemoveEvent>(relative_path));
	}

	void EventFilter::ConflictFilter(std::shared_ptr<EventQueue> event_queue, const std::wstring& relative_path) const {
		if (common_utility::HasConflict(relative_path)) {
			return;
		}

		std::wclog << L"EventFilter ConflictEvent: " << relative_path << std::endl;
		event_queue->Push(std::make_unique<ConflictEvent>(relative_path));
	}
}  // namespace monitor_client