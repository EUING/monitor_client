#ifndef MONITOR_CLIENT_ITEM_SQL_H_
#define MONITOR_CLIENT_ITEM_SQL_H_

#include <optional>
#include <string>

#include "item_dao.h"
#include "common_utility.h"

namespace monitor_client {
	class ItemSql {
	public:
		ItemSql(ItemDao* item_dao);

		ItemSql(const ItemSql&) = delete;
		ItemSql& operator=(const ItemSql&) = delete;

		ItemSql(ItemSql&&);
		ItemSql& operator=(ItemSql&&);

		~ItemSql() = default;

		std::optional<int> GetParentId(const std::wstring& relative_path);
		std::optional<common_utility::FileInfo> GetFileInfo(const std::wstring& relative_path);

		bool RenameItem(const common_utility::ChangeNameInfo& name_info);
		bool RemoveItem(const std::wstring& relative_path);

		bool AddFile(const common_utility::FileInfo& info);
		bool ModifyFile(const common_utility::FileInfo& info);

		bool AddFolder(const common_utility::FolderInfo& info);

	private:
		ItemDao* item_dao_;
	};
}  // namespace monitor_client
#endif
