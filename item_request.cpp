#include "item_request.h"

#include <memory>

#include "item_http.h"
#include "local_db.h"

namespace monitor_client {
	ItemRequest::ItemRequest(const common_utility::NetworkInfo& network_info, std::unique_ptr<ItemDao>&& item_dao) : 
		item_http_(std::make_shared<ItemHttp>(network_info)), 
		local_db_(std::move(item_dao)) {
	}

	ItemRequest::ItemRequest(const std::shared_ptr<ItemHttp>& item_http, LocalDb&& local_db) :
		item_http_(item_http),
		local_db_(std::move(local_db)) {
	}

	bool ItemRequest::UploadRequest(const common_utility::ItemInfo& item_info) {
		if (!item_http_) {
			std::wcerr << L"ItemRequest::UploadRequest: item_http_ is null" << std::endl;
			return false;
		}

		if (!item_http_->UpdateItem(item_info)) {
			std::wcerr << L"ItemRequest::UploadRequest: item_http_.UpdateItem Fail: " << item_info.name << std::endl;
			return false;
		}

		if (!local_db_.UpdateItem(item_info)) {
			std::wcerr << L"ItemRequest::UploadRequest: local_db_.UpdateItem Fail: " << item_info.name << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::DownloadRequest(const std::wstring& relative_path) {
		if (!item_http_) {
			std::wcerr << L"ItemRequest::DownloadRequest: item_http_ is null" << std::endl;
			return false;
		}

		std::optional<common_utility::ItemInfo> item_info_opt = item_http_->GetItemInfo(relative_path);
		if (!item_info_opt.has_value()) {
			std::wcerr << L"ItemRequest::DownloadRequest: item_http_.GetItemInfo Fail: " << relative_path << std::endl;
			return false;
		}

		auto item_info = item_info_opt.value();
		item_info.name = relative_path;

		if (!local_db_.UpdateItem(item_info)) {
			std::wcerr << L"ItemRequest::DownloadRequest: local_db_.UpdateItem Fail: " << relative_path << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::RenameRequest(const common_utility::ChangeNameInfo& change_name_info) {
		if (!item_http_) {
			std::wcerr << L"ItemRequest::RenameRequest: item_http_ is null" << std::endl;
			return false;
		}

		if (!item_http_->RenameItem(change_name_info)) {
			std::wcerr << L"ItemRequest::RenameRequest: item_http_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
			return false;
		}

		if (!local_db_.RenameItem(change_name_info)) {
			std::wcerr << L"ItemRequest::RenameRequest: local_db_.RenameItem Fail: " << change_name_info.old_name << L'?' << change_name_info.new_name << std::endl;
			return false;
		}

		return true;
	}

	bool ItemRequest::RemoveRequest(const std::wstring& relative_path) {
		if (!item_http_) {
			std::wcerr << L"ItemRequest::RemoveRequest: item_http_ is null" << std::endl;
			return false;

		}
		if (!item_http_->RemoveItem(relative_path)) {
			std::wcerr << L"ItemRequest::RemoveRequest: item_http_.RemoveItem Fail: " << relative_path << std::endl;
			return false;
		}

		if (!local_db_.RemoveItem(relative_path)) {
			std::wcerr << L"ItemRequest::RemoveRequest: local_db_.RemoveItem Fail: " << relative_path << std::endl;
			return false;
		}

		return true;
	}
	bool ItemRequest::LocalRemoveRequest(const std::wstring& relative_path) {
		if (!local_db_.RemoveItem(relative_path)) {
			std::wcerr << L"ItemRequest::RemoveRequest: local_db_.RemoveItem Fail: " << relative_path << std::endl;
			return false;
		}

		return true;
	}
}  // namespace monitor_client