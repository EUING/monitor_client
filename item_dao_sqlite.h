#ifndef MONITOR_CLIENT_ITEM_DAO_SQLITE_H_
#define MONITOR_CLIENT_ITEM_DAO_SQLITE_H_

#include <optional>
#include <vector>
#include <string>
#include <memory>

#include "sqlite_wrapper.h"

#include "item_dao.h"
#include "common_utility.h"

namespace monitor_client {
	class ItemDaoSqlite : public ItemDao {
	public:
		ItemDaoSqlite() = default;

		ItemDaoSqlite(const ItemDaoSqlite&) = delete;
		ItemDaoSqlite& operator=(const ItemDaoSqlite&) = delete;

		ItemDaoSqlite(ItemDaoSqlite&&) = default;
		ItemDaoSqlite& operator=(ItemDaoSqlite&&) = default;

		virtual ~ItemDaoSqlite() = default;

		bool OpenDatabase(const std::wstring& database_path) override;
		std::optional<int> GetItemId(const std::wstring& item_name, int parent_id) const override;
		std::optional<common_utility::FileInfo> GetFileInfo(const std::wstring& file_name, int parent_id) const override;
		std::optional<std::vector<common_utility::FileInfo>> GetFolderContainList(int parent_id) const override;

		std::optional<int> ChangeItemName(const common_utility::ChangeNameInfo& name_info, int parent_id) override;
		std::optional<int> DeleteItemInfo(const std::wstring& item_name, int parent_id) override;
		std::optional<int> InsertFileInfo(const common_utility::FileInfo& file_info, int parent_id) override;
		std::optional<int> ModifyFileInfo(const common_utility::FileInfo& file_info, int parent_id) override;
		std::optional<int> InsertFolderInfo(const common_utility::FolderInfo& folder_info, int parent_id) override;

	private:
		std::unique_ptr<sqlite_manager::utf16::SqliteWrapper> sqlite_wrapper_;
	};
}  // namespace monitor_client
#endif
