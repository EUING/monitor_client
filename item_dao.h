#ifndef MONITOR_CLIENT_ITEM_DAO_H_
#define MONITOR_CLIENT_ITEM_DAO_H_

#include <optional>
#include <memory>

#include "sqlite_wrapper.h"

#include "common_utility.h"

namespace monitor_client {
	class ItemDao {
	public:
		ItemDao() = default;

		ItemDao(const ItemDao&) = delete;
		ItemDao& operator=(const ItemDao&) = delete;

		ItemDao(ItemDao&&) = default;
		ItemDao& operator=(ItemDao&&) = default;

		~ItemDao() = default;

		bool OpenDatabase(const std::wstring& database_path);
		std::optional<int> GetItemId(const std::wstring& item_name, int parent_id);
		std::optional<common_utility::FileInfo> GetFileInfo(const std::wstring& file_name, int parent_id);
		std::optional<std::vector<common_utility::FileInfo>> GetFolderContainList(int parent_id);

		std::optional<int> ChangeItemName(const common_utility::ChangeNameInfo& name_info, int parent_id);
		std::optional<int> DeleteItemInfo(const std::wstring& item_name, int parent_id);
		std::optional<int> InsertFileInfo(const common_utility::FileInfo& file_info, int parent_id);
		std::optional<int> ModifyFileInfo(const common_utility::FileInfo& file_info, int parent_id);
		std::optional<int> InsertFolderInfo(const common_utility::FolderInfo& folder_info, int parent_id);

	private:
		std::unique_ptr<sqlite_manager::utf16::SqliteWrapper> sqlite_wrapper_;
	};
}  // namespace monitor_client
#endif
