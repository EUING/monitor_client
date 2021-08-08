#include "item_request.h"

#include <memory>
#include <optional>

#include "common_utility.h"
#include "item_http.h"
#include "local_db.h"
#include "item_s3.h"

namespace monitor_client {
	ItemRequest::ItemRequest(const std::shared_ptr<ItemHttp>& item_http, const std::shared_ptr<ItemS3>& item_s3, const std::shared_ptr<LocalDb>& local_db) :
		item_http_(item_http),
		item_s3_(item_s3),
		local_db_(local_db) {
	}

	bool ItemRequest::UploadRequest(const common_utility::ItemInfo& item_info) {
		std::optional<bool> is_valid = common_utility::IsValidItem(item_info);
		if (!is_valid.has_value()) {
			std::wcerr << L"ItemRequest::UploadRequest: IsValidItem Fail: " << item_info.name << std::endl;
			return false;
		}

		if (!is_valid.value()) {
			std::wclog << item_info.name << " is changed: Skip this event" << std::endl;
			return true;  // 파일 정보가 계속 갱신되고 있으면 Skip
		}
		
		if (item_info.size >= 0) {
			if (item_s3_ && !item_s3_->PutItem(item_info)) {
				std::wcerr << L"ItemRequest::UploadRequest: item_s3_.PutItem Fail: " << item_info.name << std::endl;
				return false;
			}
		}

		if (!item_http_->UpdateItem(item_info)) {
			std::wcerr << L"ItemRequest::UploadRequest: item_http_.UpdateItem Fail: " << item_info.name << std::endl;
			return false;
		}

		if (!local_db_->UpdateItem(item_info)) {
			std::wcerr << L"ItemRequest::UploadRequest: local_db_.UpdateItem Fail: " << item_info.name << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::DownloadRequest(const std::wstring& relative_path) {
		std::optional<common_utility::ItemInfo> item_info_opt = item_http_->GetItemInfo(relative_path);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"ItemRequest::DownloadRequest: item_http_.GetItemInfo Fail: " << relative_path << std::endl;
			return false;
		}

		auto item_info = item_info_opt.value();
		item_info.name = relative_path;

		if (item_info.size < 0) {
			_wmkdir(item_info.name.c_str());
		}
		else {
			if (item_s3_ && !item_s3_->GetItem(item_info)) {
				std::wcerr << L"ItemRequest::DownloadRequest: item_s3_.GetItem Fail: " << relative_path << std::endl;
				return false;
			}
		}

		if (!local_db_->UpdateItem(item_info)) {
			std::wcerr << L"ItemRequest::DownloadRequest: local_db_.UpdateItem Fail: " << relative_path << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::RenameRequest(const common_utility::ChangeNameInfo& change_name_info) {
		if (!item_http_->RenameItem(change_name_info)) {
			std::wcerr << L"ItemRequest::RenameRequest: item_http_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
			return false;
		}

		if (!local_db_->RenameItem(change_name_info)) {
			std::wcerr << L"ItemRequest::RenameRequest: local_db_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::RemoveRequest(const std::wstring& relative_path) {
		if (!item_http_->RemoveItem(relative_path)) {
			std::wcerr << L"ItemRequest::RemoveRequest: item_http_.RemoveItem Fail: " << relative_path << std::endl;
			return false;
		}

		if (!local_db_->RemoveItem(relative_path)) {
			std::wcerr << L"ItemRequest::RemoveRequest: local_db_.RemoveItem Fail: " << relative_path << std::endl;
			return false;
		}

		return true;
	}
	bool ItemRequest::LocalRemoveRequest(const std::wstring& relative_path) {
		if (!local_db_->RemoveItem(relative_path)) {
			std::wcerr << L"ItemRequest::RemoveRequest: local_db_.RemoveItem Fail: " << relative_path << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::LocalRenameRequest(const common_utility::ChangeNameInfo& change_name_info) {
		if (!local_db_->RenameItem(change_name_info)) {
			std::wcerr << L"ItemRequest::RenameRequest: local_db_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
			return false;
		}

		return true;
	}
}  // namespace monitor_client