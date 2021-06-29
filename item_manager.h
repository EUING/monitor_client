#ifndef MONITOR_CLIENT_ITEM_MANAGER_H_
#define MONITOR_CLIENT_ITEM_MANAGER_H_

#include <optional>
#include <future>
#include <string>

#include "item_http.h"
#include "notify_queue.h"

namespace monitor_client {
	class ItemManager {
	public:
		ItemManager(NotifyQueue* notify_queue, ItemHttp* item_http);

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
		NotifyQueue* notify_queue_;
		ItemHttp* item_http_;
	};
}  // namespace monitor_client
#endif