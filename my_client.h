#ifndef REST_CLIENT_MY_CLIENT_H_
#define REST_CLIENT_MY_CLIENT_H_

#include <optional>
#include <string>

#include<cpprest/http_client.h>

namespace my_rest_client {
	std::optional<std::wstring> Ping();
}  // namespace my_rest_client
#endif
