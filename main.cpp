#include <atlstr.h>

#include <iostream>
#include <string>
#include <memory>
#include <optional>

#include "event_filter.h"
#include "common_utility.h"
#include "diff_check.h"
#include "item_http.h"
#include "local_db.h"
#include "event_queue.h"
#include "custom_event_pusher.h"
#include "event_consumer.h"
#include "item_dao_sqlite.h"
#include "folder_watcher.h"
#include "yaml-cpp/yaml.h"

const wchar_t* const kConfigFile = L"config.yaml";

int main(int argc, char* argv[]) {
	std::wcin.imbue(std::locale("Korean"));
	std::wcout.imbue(std::locale("Korean"));
	std::wcerr.imbue(std::locale("Korean"));
	std::wclog.imbue(std::locale("Korean"));

	if (argc < 2) {
		std::wcerr << L"Please enter folder path" << std::endl;
		return -1;
	}

	std::wstring folder_path;
	for (int i = 1; i < argc; ++i) {
		folder_path.append(CA2W(argv[i]).m_psz).append(L" ");
	}

	folder_path.pop_back();

	if (!SetCurrentDirectory(folder_path.c_str())) {
		std::wcerr << L"Please check folder path: " << folder_path << std::endl;
		return -1;
	}

	std::wstring ignore_path = folder_path;

	ignore_path.append(L"\\.ignore");
	std::optional<bool> is_dir =  monitor_client::common_utility::IsDirectory(ignore_path);
	if (!(is_dir.has_value() && is_dir.value())) {
		std::wcerr << L"Please check ignore folder path: " << ignore_path << std::endl;
		return -1;
	}

	std::wstring yaml_path = ignore_path + L"\\" + kConfigFile;
	std::ifstream is(yaml_path);
	if (!is) {
		std::wcerr << L"Please check yaml file: " << yaml_path << std::endl;
		return -1;
	}
	is.close();

	YAML::Node node = YAML::LoadFile(CW2A(yaml_path.c_str()).m_psz);
	if (!(node["database"].IsDefined() && node["database"].IsScalar())) {
		std::wcerr << L"Please check \"database\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	if (!(node["network"].IsDefined() && node["network"].IsMap())) {
		std::wcerr << L"Please check \"network\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	if (!(node["network"]["host"].IsDefined() && node["network"]["host"].IsScalar())) {
		std::wcerr << L"Please check \"network::host\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	if (!(node["network"]["port"].IsDefined() && node["network"]["port"].IsScalar())) {
		std::wcerr << L"Please check \"network::port\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	std::wstring db = CA2W(node["database"].as<std::string>().c_str(), CP_UTF8).m_psz;
	if (std::wstring::npos == db.find(L".db")) {
		db.append(L".db");
	}

	std::wstring host = CA2W(node["network"]["host"].as<std::string>().c_str(), CP_UTF8).m_psz;
	std::wstring port = CA2W(node["network"]["port"].as<std::string>().c_str(), CP_UTF8).m_psz;

	if (port.end() != std::find_if(port.begin(), port.end(), [](wchar_t c) { return !iswdigit(c); })) {
		std::wcerr << L"Please check \"network::port\" value is number in yaml: " << yaml_path << std::endl;
		return -1;
	}

	std::wstring db_path = ignore_path + L"\\" + db;
	auto sqlite = std::make_unique<monitor_client::ItemDaoSqlite>();
	if (!sqlite->OpenDatabase(db_path)) {
		return -1;
	}
	
	std::shared_ptr<monitor_client::LocalDb> local_db = std::make_shared<monitor_client::LocalDb>(std::move(sqlite));
	monitor_client::common_utility::NetworkInfo network_info{ host, _wtoi(port.c_str()) };
	std::shared_ptr<monitor_client::ItemHttp> item_http = std::make_shared<monitor_client::ItemHttp>(network_info);

	monitor_client::diff_check::ServerDiffList server_diff_list = monitor_client::diff_check::InitialDiffCheck(local_db, item_http);

	auto event_queue = std::make_shared<monitor_client::EventQueue>();
	monitor_client::EventConsumer event_consumer(event_queue, item_http, local_db);	
	if (!event_consumer.Run()) {
		return -1;
	}

	std::shared_ptr<monitor_client::BaseEventFilter> event_filter = std::make_shared<monitor_client::EventFilter>(local_db);
	monitor_client::EventProducer event_producer(event_filter, event_queue);
	event_producer.PushEvent(std::make_unique<monitor_client::CustomEventPusher>(server_diff_list));
	
	monitor_client::FolderWatcher folder_watcher(event_producer, folder_path);
	if (!folder_watcher.StartWatching()) {
		return -1;
	}

	wchar_t c = L'\0';
	while (std::wcin >> c) {

	}

	return 0;
}