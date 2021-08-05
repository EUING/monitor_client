#ifndef MONITOR_CLIENT_YAML_PARSER_H_
#define MONITOR_CLIENT_YAML_PARSER_H_

#include <atlstr.h>

#include <optional>
#include <vector>
#include <string>
#include <iostream>

#include "yaml-cpp/yaml.h"

namespace monitor_client {
	class YamlParser {
	public:
		YamlParser(const std::wstring& yaml_path) : node_() {
			node_ = YAML::LoadFile(CW2A(yaml_path.c_str()).m_psz);
		}

		YamlParser(const YamlParser&) = default;
		YamlParser& operator=(const YamlParser&) = default;

		YamlParser(YamlParser&&) = default;
		YamlParser& operator=(YamlParser&&) = default;

		~YamlParser() = default;

		template<typename ... Args>
		std::optional<std::wstring> GetString(Args... args) const {
			std::vector<std::string> arg_list;
			(arg_list.push_back(args), ...);
			
			if (arg_list.empty()) {
				std::wcerr << L"YamlParser::GetString: Wrong Args" << std::endl;
				return std::nullopt;
			}

			YAML::Node node = YAML::Clone(node_);
			int size = arg_list.size();
			for (int i = 0; i < size - 1; ++i) {
				if (!node[arg_list[i]].IsDefined() || !node[arg_list[i]].IsMap()) {
					return std::nullopt;
				}

				node = node[arg_list[i]];
			}

			if (!node[arg_list[size - 1]].IsDefined() || !node[arg_list[size - 1]].IsScalar()) {
				return std::nullopt;
			}

			return CA2W(node[arg_list[size - 1]].as<std::string>().c_str(), CP_UTF8).m_psz;
		}

		template<typename ... Args>
		std::optional<int> GetInt(Args... args) const {
			std::optional<std::wstring> get_number = GetString(std::forward<Args>(args)...);
			if (!get_number.has_value()) {
				return std::nullopt;
			}

			auto number = get_number.value();
			if (number.end() != std::find_if(number.begin(), number.end(), [](wchar_t c) { return !iswdigit(c); })) {
				return std::nullopt;
			}

			return _wtoi(number.c_str());
		}

	private:
		YAML::Node node_;
	};
}  // namespace monitor_client
#endif