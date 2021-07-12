#ifndef MONITOR_CLIENT_DOWNLOAD_EVENT_H_
#define MONITOR_CLIENT_DOWNLOAD_EVENT_H_

#include <string>

#include "base_event.h"
#include "item_request.h"

namespace monitor_client {
	class DownloadEvent : public BaseEvent {
	public:
		DownloadEvent(const std::wstring& relative_path) : BaseEvent(), relative_path_(relative_path) {}

		DownloadEvent(const DownloadEvent&) = default;
		DownloadEvent& operator=(const DownloadEvent&) = default;

		DownloadEvent(DownloadEvent&&) = default;
		DownloadEvent& operator=(DownloadEvent&&) = default;

		~DownloadEvent() override = default;

		bool operator()(ItemRequest& item_request) const override;

	private:
		std::wstring relative_path_;
	};
}  // namespace monitor_client
#endif