#ifndef MONITOR_CLIENT_COMMON_STRUCT_H_
#define MONITOR_CLIENT_COMMON_STRUCT_H_

#include <stdint.h>

#include <string>

namespace monitor_client {
namespace common_utility {
	struct ItemInfo {
		std::wstring name;
		int64_t size;
		std::wstring hash;

		friend bool operator==(const ItemInfo& lhs, const ItemInfo& rhs) {
			return lhs.name == rhs.name;
		}
	};

	struct ChangeNameInfo {
		std::wstring old_name;
		std::wstring new_name;
	};

	struct NetworkInfo {
		std::wstring host;
		int port;
	};

	struct S3Info {
		std::wstring id;
		std::wstring password;
		std::wstring bucket;
	};
}  // namespace common_utility
}  // namespace monitor_client

namespace std {
	template<>
	struct hash<monitor_client::common_utility::ItemInfo> {
		size_t operator() (const monitor_client::common_utility::ItemInfo& info) const noexcept {
			hash<wstring> hash_func;
			return hash_func(info.name);
		}
	};
}  // namespace std
#endif