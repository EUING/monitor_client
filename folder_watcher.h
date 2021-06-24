#ifndef REST_CLIENT_FOLDER_WATCHER_H_
#define REST_CLIENT_FOLDER_WATCHER_H_

#include <Windows.h>

#include <future>
#include <optional>
#include <string>

#include "notify_queue.h"

namespace monitor_client {
	class FolderWatcher {
	public:
		FolderWatcher(NotifyQueue* notify_queue, const std::wstring& watch_folder = L"");

		FolderWatcher(const FolderWatcher&) = delete;
		FolderWatcher& operator=(const FolderWatcher&) = delete;

		FolderWatcher(FolderWatcher&&) = delete;
		FolderWatcher& operator=(FolderWatcher&&) = delete;

		~FolderWatcher();

		bool IsRunning();
		void StopWatching();
		bool StartWatching();
		std::wstring GetWatchFolder() { return watch_folder_; }
		void SetWatchFolder(const std::wstring& watch_folder) { watch_folder_ = watch_folder; }

	private:
		bool InitWatching(std::shared_ptr<void>& folder_handle_ptr, std::shared_ptr<void>& overlap_event_ptr);
		void WatchingDirectory(std::shared_ptr<void> folder_handle_ptr, std::shared_ptr<void> overlap_event_ptr);
		void CloseEvent();

		std::future<void> thread_future_;
		HANDLE stop_watching_event_;  // 감시 종료 이벤트
		std::wstring watch_folder_;  // 감시하는 경로
		NotifyQueue* notify_queue_;
	};

}  // namespace monitor_client
#endif