#ifndef MONITOR_CLIENT_INITIAL_DIFF_CHECKER_H_
#define MONITOR_CLIENT_INITIAL_DIFF_CHECKER_H_

#include <memory>

#include "local_db.h"
#include "item_http.h"
#include "diff_check.h"

namespace monitor_client {
	class InitialDiffChecker {
	public:
		InitialDiffChecker(const std::shared_ptr<LocalDb>& local_db, const std::shared_ptr<ItemHttp>& item_http);

		InitialDiffChecker(const InitialDiffChecker&) = default;
		InitialDiffChecker& operator=(const InitialDiffChecker&) = default;

		InitialDiffChecker(InitialDiffChecker&&) = default;
		InitialDiffChecker& operator=(InitialDiffChecker&&) = default;

		~InitialDiffChecker() = default;

		diff_check::ServerDiffList FindDifference(const std::wstring& relative_path = L"") const;

	private:
		std::shared_ptr<LocalDb> local_db_;
		std::shared_ptr<ItemHttp> item_http_;
	};
}  // namespace monitor_client
#endif