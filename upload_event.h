#ifndef MONITOR_CLIENT_UPLOAD_EVENT_H_
#define MONITOR_CLIENT_UPLOAD_EVENT_H_

#include "common_struct.h"
#include "base_event.h"
#include "item_request.h"

namespace monitor_client {
	class UploadEvent : public BaseEvent {
	public:
		UploadEvent(const common_utility::ItemInfo& item_info) : BaseEvent(), item_info_(item_info) {}

		UploadEvent(const UploadEvent&) = default;
		UploadEvent& operator=(const UploadEvent&) = default;

		UploadEvent(UploadEvent&&) = default;
		UploadEvent& operator=(UploadEvent&&) = default;

		~UploadEvent() override = default;

		bool Execute(ItemRequest& item_request) const override;

	private:
		common_utility::ItemInfo item_info_;
	};
}  // namespace monitor_client
#endif