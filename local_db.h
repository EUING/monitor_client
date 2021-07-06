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
		std::optional<common_utility::FileInfo> GetFileInfo(const std::wstring& relative_path) const;

		bool RenameItem(const common_utility::ChangeNameInfo& name_info);
		bool RemoveItem(const std::wstring& relative_path);

		bool AddFile(const common_utility::FileInfo& info);
		bool ModifyFile(const common_utility::FileInfo& info);

		bool AddFolder(const common_utility::FolderInfo& info);

	private:
		std::unique_ptr<ItemDao> item_dao_;
	};
}  // namespace monitor_client
#endif
