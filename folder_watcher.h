#ifndef REST_CLIENT_FOLDER_WATCHER_H_
#define REST_CLIENT_FOLDER_WATCHER_H_

#include <optional>
#include <string>

namespace my_rest_client {
	extern bool g_watching;

	std::optional<bool> WatchDirectory(const std::wstring& folder_name);
}  // namespace my_rest_client
#endif