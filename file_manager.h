#ifndef REST_CLIENT_FILE_MANAGER_H_
#define REST_CLIENT_FILE_MANAGER_H_

#include <future>

#include "change_info_queue.h"

namespace my_rest_client {
	class FileManager {
	public:
		FileManager(ChangeInfoQueue* change_info);

		FileManager(const FileManager&) = delete;
		FileManager& operator=(const FileManager&) = delete;

		FileManager(FileManager&&) = delete;
		FileManager& operator=(FileManager&&) = delete;

		~FileManager();

		bool Run();
		void Stop();
		bool IsRunning();

	private:
		bool Management(const ChangeInfo& info);

		std::future<void> thread_future_;
		ChangeInfoQueue* change_info_;
	};
}  // namespace my_rest_client
#endif