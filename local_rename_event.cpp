#include "local_rename_event.h"

#include "item_request.h"

namespace monitor_client {
	bool LocalRenameEvent::Execute(ItemRequest& item_request) const {
		if (0 != _wrename(change_name_info_.old_name.c_str(), change_name_info_.new_name.c_str())) {
			std::wcerr << L"LocalRenameEvent::Execute: _wrename Fail: " << change_name_info_.old_name << L'?' << change_name_info_.new_name << std::endl;
			return false;
		}

		return item_request.LocalRenameRequest(change_name_info_);
	}
}  // namespace monitor_client