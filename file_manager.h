#ifndef REST_CLIENT_FILE_MANAGER_H_
#define REST_CLIENT_FILE_MANAGER_H_

#include <optional>
#include <future>
#include <string>

#include "notify_queue.h"

namespace monitor_client {
	class FileManager {
	public:
		FileManager(NotifyQueue* notify_queue);

		FileManager(const FileManager&) = delete;
		FileManager& operator=(const FileManager&) = delete;

		FileManager(FileManager&&) = delete;
		FileManager& operator=(FileManager&&) = delete;

		~FileManager();

		bool Run();
		void Stop();
		bool IsRunning();

	private:
		std::future<void> thread_future_;
		NotifyQueue* notify_queue_;
	};
}  // namespace monitor_client
#endif