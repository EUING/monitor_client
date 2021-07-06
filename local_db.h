#ifndef MONITOR_CLIENT_LOCAL_DB_H_
#define MONITOR_CLIENT_LOCAL_DB_H_

#include <optional>
#include <string>
#include <memory>

#include "item_dao.h"
#include "common_utility.h"

namespace monitor_client {
	class LocalDb {
	public:
		explicit LocalDb(std::unique_ptr<ItemDao>&& item_dao);

		LocalDb(const LocalDb&) = delete;
		LocalDb& operator=(const LocalDb&) = delete;

		LocalDb(LocalDb&&);
		LocalDb& operator=(LocalDb&&);

		~LocalDb() = default;

		std::optional<int> GetParentId(const std::wstring& relative_path) const;
		std::optional<common_utility::ItemInfo> GetItemInfo(const std::wstring& relative_path) const;

		bool InsertItem(const common_utility::ItemInfo& item_info);
		bool RenameItem(const common_utility::ChangeNameInfo& name_info);
		bool RemoveItem(const std::wstring& relative_path);

	private:
		std::unique_ptr<ItemDao> item_dao_;
	};
}  // namespace monitor_client
#endif
