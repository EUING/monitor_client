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
		bool StartWatching(const std::wstring& folder_path);		

	private:
		std::optional<std::shared_ptr<void>> InitWatching(const std::wstring& folder_path);
		void WatchingDirectory(std::shared_ptr<void> folder_handle_ptr);
		void CloseEvent();

		std::shared_future<void> thread_future_;
		HANDLE stop_watching_event_;  // ���� ���� �̺�Ʈ
	};

}  // namespace my_rest_client
#endif