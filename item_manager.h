#ifndef MONITOR_CLIENT_ITEM_MANAGER_H_
#define MONITOR_CLIENT_ITEM_MANAGER_H_

#include <optional>
#include <memory>
#include <future>
#include <string>

#include "common_utility.h"
#include "item_http.h"
#include "local_db.h"
#include "notify_queue.h"

namespace monitor_client {
	class ItemManager {
	public:
		ItemManager(std::shared_ptr<NotifyQueue> notify_queue, const common_utility::NetworkInfo& network_info, std::unique_ptr<ItemDao>&& item_dao);

		ItemManager(const ItemManager&) = delete;
		ItemManager& operator=(const ItemManager&) = delete;

		ItemManager(ItemManager&&) = delete;
		ItemManager& operator=(ItemManager&&) = delete;

		~ItemManager();

		bool Run();
		void Stop();
		bool IsRunning();
		void ManagementItem(const common_utility::ChangeItemInfo& info);
		void ManagementSubFolder(const std::wstring& relative_path);

	private:
		std::future<void> thread_future_;
		std::shared_ptr<NotifyQueue> notify_queue_;
		ItemHttp item_http_;
		LocalDb local_db_;
	};
}  // namespace monitor_client
#endif