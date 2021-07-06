#ifndef MONITOR_CLIENT_ITEM_DAO_H_
#define MONITOR_CLIENT_ITEM_DAO_H_

#include <optional>

#include "common_utility.h"

namespace monitor_client {
	class ItemDao {
	public:
		ItemDao() = default;

		ItemDao(const ItemDao&) = delete;
		ItemDao& operator=(const ItemDao&) = delete;

		ItemDao(ItemDao&&) = default;
		ItemDao& operator=(ItemDao&&) = default;

		virtual ~ItemDao() = default;

		virtual bool OpenDatabase(const std::wstring& database_path) = 0;
		virtual std::optional<int> GetItemId(const std::wstring& item_name, int parent_id) const = 0;
		virtual std::optional<common_utility::ItemInfo> GetItemInfo(const std::wstring& file_name, int parent_id) const = 0;
		virtual std::optional<std::vector<common_utility::ItemInfo>> GetFolderContainList(int parent_id) const = 0;

		virtual std::optional<int> ChangeItemName(const common_utility::ChangeNameInfo& name_info, int parent_id) = 0;
		virtual std::optional<int> DeleteItemInfo(const std::wstring& item_name, int parent_id) = 0;
		virtual std::optional<int> InsertItemInfo(const common_utility::ItemInfo& item_info, int parent_id) = 0;
		virtual std::optional<int> ModifyItemInfo(const common_utility::ItemInfo& item_info, int parent_id) = 0;
	};

	class ItemDaoDummy : public monitor_client::ItemDao {
		bool OpenDatabase(const std::wstring& database_path) override { return true; }
		std::optional<int> GetItemId(const std::wstring& item_name, int parent_id) const override { return 0; }
		std::optional<monitor_client::common_utility::ItemInfo> GetItemInfo(const std::wstring& item_name, int parent_id) const override { return monitor_client::common_utility::ItemInfo(); }
		std::optional<std::vector<monitor_client::common_utility::ItemInfo>> GetFolderContainList(int parent_id) const override { return std::vector<monitor_client::common_utility::ItemInfo>(); }
		std::optional<int> ChangeItemName(const monitor_client::common_utility::ChangeNameInfo& name_info, int parent_id) override { return 1; }
		std::optional<int> DeleteItemInfo(const std::wstring& item_name, int parent_id) override { return 1; }
		std::optional<int> InsertItemInfo(const monitor_client::common_utility::ItemInfo& item_info, int parent_id) override { return 1; }
		std::optional<int> ModifyItemInfo(const monitor_client::common_utility::ItemInfo& item_info, int parent_id) override { return 1; }
	};
}  // namespace monitor_client
#endif