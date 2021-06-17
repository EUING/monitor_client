#include "my_client.h"

#include <optional>
#include <string>

#include<cpprest/http_client.h>

namespace my_rest_client {
	std::optional<std::wstring> Ping() {
		web::http::client::http_client client(L"http://ggulmo.iptime.org:56380/ping");

		std::optional<std::wstring> result;
		client.request(web::http::methods::GET).then([&result](web::http::http_response response) {
				if (response.status_code() == web::http::status_codes::OK) {
					std::wstring body = response.extract_string().get();
					if (body == L"pong") {
						result = body;
					}
				}
			}
		).wait();

		return result;
	}
}  // namespace my_rest_client