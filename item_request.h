#ifndef MONITOR_CLIENT_ITEM_REQUEST_H_
#define MONITOR_CLIENT_ITEM_REQUEST_H_

#include <memory>
#include <string>

#include "common_struct.h"
#include "item_http.h"
#include "local_db.h"
#include "item_s3.h"

namespace monitor_client {
	class ItemRequest {
	public:
		ItemRequest(const std::shared_ptr<ItemHttp>& item_http, const std::shared_ptr<ItemS3>& item_s3, const std::shared_ptr<LocalDb>& local_db);

		ItemRequest(const ItemRequest&) = delete;
		ItemRequest& operator=(const ItemRequest&) = delete;

		ItemRequest(ItemRequest&&) = default;
		ItemRequest& operator=(ItemRequest&&) = default;

		~ItemRequest() = default;

		bool UploadRequest(const common_utility::ItemInfo& item_info);
		bool DownloadRequest(const std::wstring& relative_path);
		bool RenameRequest(const common_utility::ChangeNameInfo& change_name_info);
		bool RemoveRequest(const std::wstring& relative_path);
		bool LocalRemoveRequest(const std::wstring& relative_path);
		bool LocalRenameRequest(const common_utility::ChangeNameInfo& change_name_info);

	private:
		std::shared_ptr<ItemHttp> item_http_;
		std::shared_ptr<ItemS3> item_s3_;
		std::shared_ptr<LocalDb> local_db_;
	};
}  // namespace monitor_client
#endif