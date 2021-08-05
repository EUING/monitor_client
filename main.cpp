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
#include "item_s3.h"
#include "yaml_parser.h"
#include "web_socket.h"

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

	monitor_client::YamlParser parser(yaml_path);
	std::optional<std::wstring> get_db = parser.GetString("database");
	if (!get_db.has_value()) {
		std::wcerr << L"Please check \"database\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto db = get_db.value();
	if (std::wstring::npos == db.find(L".db")) {
		db.append(L".db");
	}

	std::optional<std::wstring> get_host = parser.GetString("network", "host");
	if (!get_host.has_value()) {
		std::wcerr << L"Please check \"network::host\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto http_host = get_host.value();

	std::optional<int> get_port = parser.GetInt("network", "port");
	if (!get_port.has_value()) {
		std::wcerr << L"Please check \"network::port\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto http_port = get_port.value();

	get_host = parser.GetString("minio", "host");
	if (!get_host.has_value()) {
		std::wcerr << L"Please check \"minio::host\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto minio_host = get_host.value();

	get_port = parser.GetInt("minio", "port");
	if (!get_port.has_value()) {
		std::wcerr << L"Please check \"minio::port\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto minio_port = get_port.value();

	std::optional<std::wstring> get_id = parser.GetString("minio", "id");
	if (!get_id.has_value()) {
		std::wcerr << L"Please check \"minio::id\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto minio_id = get_id.value();

	std::optional<std::wstring> get_password = parser.GetString("minio", "password");
	if (!get_password.has_value()) {
		std::wcerr << L"Please check \"minio::password\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto minio_password = get_password.value();

	std::optional<std::wstring> get_bucket = parser.GetString("minio", "bucket");
	if (!get_bucket.has_value()) {
		std::wcerr << L"Please check \"minio::bucket\" key in yaml: " << yaml_path << std::endl;
		return -1;
	}

	auto minio_bucket = get_bucket.value();

	monitor_client::common_utility::NetworkInfo http_info{ http_host, http_port};
	std::shared_ptr<monitor_client::ItemHttp> item_http = std::make_shared<monitor_client::ItemHttp>(http_info);	

	std::wstring db_path = ignore_path + L"\\" + db;
	auto sqlite = std::make_unique<monitor_client::ItemDaoSqlite>();
	if (!sqlite->OpenDatabase(db_path)) {
		return -1;
	}

	std::shared_ptr<monitor_client::LocalDb> local_db = std::make_shared<monitor_client::LocalDb>(std::move(sqlite));	

	monitor_client::diff_check::ServerDiffList server_diff_list = monitor_client::diff_check::InitialDiffCheck(local_db, item_http);

	monitor_client::common_utility::NetworkInfo minio_info{ minio_host, minio_port};
	monitor_client::common_utility::S3Info s3_info{ minio_id, minio_password, minio_bucket };

	std::wclog << L"Connecting to minio server..." << std::endl;
	std::shared_ptr<monitor_client::ItemS3> item_s3 = std::make_shared<monitor_client::ItemS3>(minio_info, s3_info);
	std::wclog << L"Minio server connection success" << std::endl;

	auto event_queue = std::make_shared<monitor_client::EventQueue>();
	monitor_client::EventConsumer event_consumer(event_queue, item_http, item_s3, local_db);
	if (!event_consumer.Run()) {
		return -1;
	}

	std::shared_ptr<monitor_client::BaseEventFilter> event_filter = std::make_shared<monitor_client::EventFilter>(local_db);
	monitor_client::EventProducer event_producer(event_filter, event_queue);
	event_producer.PushEvent(std::make_unique<monitor_client::CustomEventPusher>(server_diff_list));

	monitor_client::WebSocket socket(http_info, event_producer);
	if (!socket.Connect()) {
		return -1;
	}
	
	monitor_client::FolderWatcher folder_watcher(event_producer, folder_path);
	if (!folder_watcher.StartWatching()) {
		return -1;
	}

	std::wclog << L"Start to watching" << std::endl;
	wchar_t c = L'\0';
	while (std::wcin >> c) {

	}

	return 0;
}