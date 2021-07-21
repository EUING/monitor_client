#ifndef MONITOR_CLIENT_ITEM_S3_H_
#define MONITOR_CLIENT_ITEM_S3_H_

#include <memory>
#include <string>

#include "common_struct.h"

#include <aws/s3/S3Client.h>

namespace monitor_client {
	class ItemS3 {
	public:
		ItemS3(const common_utility::NetworkInfo& network_info, const common_utility::S3Info& s3_info);

		ItemS3(const ItemS3&) = delete;
		ItemS3& operator=(const ItemS3&) = delete;

		ItemS3(ItemS3&&) = default;
		ItemS3& operator=(ItemS3&&) = default;

		~ItemS3();

		bool GetItem(const common_utility::ItemInfo& item_info) const;
		bool PutItem(const common_utility::ItemInfo& item_info) const;

	private:
		std::unique_ptr<Aws::S3::S3Client> client_;
		std::wstring bucket_name_;
	};
}  // namespace monitor_client
#endif
