#include "item_s3.h"

#include <Windows.h>
#include <atlstr.h>
#undef GetMessage
#undef GetObject

#include <string>
#include <memory>
#include <fstream>

#include <aws/s3/S3Client.h>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include "common_struct.h"

namespace monitor_client {
	ItemS3::ItemS3(const common_utility::NetworkInfo& network_info, const common_utility::S3Info& s3_info) : bucket_name_(s3_info.bucket){
		Aws::SDKOptions options;
		Aws::InitAPI(options);
		
		std::wstring endpoint = network_info.host;
		endpoint.push_back(L':');
		endpoint.append(std::to_wstring(network_info.port));

		Aws::Client::ClientConfiguration config;
		config.endpointOverride = CW2A(endpoint.c_str(), CP_UTF8).m_psz;
		config.scheme = Aws::Http::Scheme::HTTP;
		config.verifySSL = false;
		
		Aws::String id = CW2A(s3_info.id.c_str(), CP_UTF8).m_psz;
		Aws::String password = CW2A(s3_info.password.c_str(), CP_UTF8).m_psz;
		client_ = std::make_unique<Aws::S3::S3Client>(Aws::Auth::AWSCredentials(id, password), config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
	}

	bool ItemS3::GetItem(const common_utility::ItemInfo& item_info) const {
		Aws::S3::Model::GetObjectRequest request;
		request.SetBucket(CW2A(bucket_name_.c_str(), CP_UTF8).m_psz);
		request.SetKey(CW2A(item_info.hash.c_str(), CP_UTF8).m_psz);

		Aws::S3::Model::GetObjectOutcome outcome = client_->GetObject(request);
		if (!outcome.IsSuccess()) {
			Aws::S3::S3Error err = outcome.GetError();
			std::wstring exception = CA2W(err.GetExceptionName().c_str(), CP_UTF8).m_psz;
			std::wstring message = CA2W(err.GetMessage().c_str(), CP_UTF8).m_psz;

			std::wcerr << L"ItemS3::GetItem: GetObjectW Fail: " << exception << L": " << message << std::endl;
			return false;
		}

		Aws::IOStream& stream = outcome.GetResultWithOwnership().GetBody();
		std::fstream file(item_info.name, std::ios::out | std::ios::binary);
		file << stream.rdbuf();
		file.close();

		return true;
	}

	bool ItemS3::PutItem(const common_utility::ItemInfo& item_info) const {
		std::ifstream is(item_info.name, std::ifstream::binary);
		if (!is) {
			std::wcerr << L"ItemS3::PutItem: ifstream Fail: " << item_info.name << std::endl;
			return false;
		}
		is.close();

		Aws::S3::Model::PutObjectRequest request;
		request.SetBucket(CW2A(bucket_name_.c_str(), CP_UTF8).m_psz);
		request.SetKey(CW2A(item_info.hash.c_str(), CP_UTF8).m_psz);

		Aws::String item_name = CW2A(item_info.name.c_str()).m_psz;
		std::shared_ptr<Aws::FStream> input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream", item_name.c_str(), std::ios_base::in | std::ios_base::binary);
		request.SetBody(input_data);

		Aws::S3::Model::PutObjectOutcome outcome = client_->PutObject(request);
		if (!outcome.IsSuccess()) {
			Aws::S3::S3Error err = outcome.GetError();
			std::wstring exception = CA2W(err.GetExceptionName().c_str(), CP_UTF8).m_psz;
			std::wstring message = CA2W(err.GetMessage().c_str(), CP_UTF8).m_psz;

			std::wcerr << L"ItemS3::PutItem: PutObject Fail: " << exception << L": " << message << std::endl;
			return false;
		}

		return true;
	}

	ItemS3::~ItemS3() {
		Aws::SDKOptions options;
		Aws::ShutdownAPI(options);
	}
}  // namespace monitor_client