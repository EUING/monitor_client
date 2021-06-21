#ifndef REST_CLIENT_FOLDER_WATCHER_H_
#define REST_CLIENT_FOLDER_WATCHER_H_

#include <windows.h>

#include <future>
#include <optional>
#include <queue>
#include <utility>
#include <string>

namespace my_rest_client {
	class FolderWatcher {
	public:
		FolderWatcher(std::queue<std::pair<DWORD, std::wstring>>* change_info, const std::wstring& watch_folder = L"");

		FolderWatcher(const FolderWatcher&) = delete;
		FolderWatcher& operator=(const FolderWatcher&) = delete;

		FolderWatcher(FolderWatcher&&);
		FolderWatcher& operator=(FolderWatcher&&);

		~FolderWatcher();

		bool IsRunning() { return stop_watching_event_ != NULL; }
		void StopWatching();
		bool StartWatching();
		std::wstring GetWatchFolder() { return watch_folder_; }
		void SetWatchFolder(const std::wstring& watch_folder) { watch_folder_ = watch_folder; }

	private:
		std::optional<std::shared_ptr<void>> InitWatching();
		void WatchingDirectory(std::shared_ptr<void> folder_handle_ptr);
		void CloseEvent();

		std::future<void> thread_future_;
		HANDLE stop_watching_event_;  // 감시 종료 이벤트
		std::wstring watch_folder_;  // 감시하는 경로
		std::queue<std::pair<DWORD, std::wstring>>* change_info_;
	};

}  // namespace my_rest_client
#endif