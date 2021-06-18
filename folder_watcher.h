#ifndef REST_CLIENT_FOLDER_WATCHER_H_
#define REST_CLIENT_FOLDER_WATCHER_H_

#include <windows.h>

#include <future>
#include <optional>
#include <string>

namespace my_rest_client {
	class FolderWatcher {
	public:
		FolderWatcher();

		FolderWatcher(const FolderWatcher&) = delete;
		FolderWatcher& operator=(const FolderWatcher&) = delete;

		FolderWatcher(FolderWatcher&&) = default;
		FolderWatcher& operator=(FolderWatcher&&) = default;

		~FolderWatcher();

		bool IsRunning() { return stop_watching_event_ != NULL; }
		void StopWatching();
		bool StartWatching(const std::wstring& watch_folder);
		std::wstring GetWatchFolder() { return watch_folder_; }

	private:
		std::optional<std::shared_ptr<void>> InitWatching(const std::wstring& watch_folder);
		void WatchingDirectory(std::shared_ptr<void> folder_handle_ptr);
		void CloseEvent();

		std::shared_future<void> thread_future_;
		HANDLE stop_watching_event_;  // 감시 종료 이벤트
		std::wstring watch_folder_;  // 감시하는 경로
	};

}  // namespace my_rest_client
#endif