#include "conflict_event.h"

#include <iostream>

#include "item_request.h"

namespace monitor_client {
	bool ConflictEvent::operator()(ItemRequest& item_request) const {
		std::wstring conflict_name = relative_path_;
		conflict_name.append(L".conflict");

		if (0 != _wrename(relative_path_.c_str(), conflict_name.c_str())) {
			std::wcerr << L"ConflictEvent::operator(): _wrename Fail: " << relative_path_ << L' ' << conflict_name << std::endl;
			return false;
		}

		return item_request.DownloadRequest(relative_path_);
	}
}  // namespace monitor_client