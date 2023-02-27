#include "reflection.h"

#include <regex>
#include <filesystem>

namespace ueditor {
	void reflection::reflect_header(assembly& assembly, const path& path) {
		std::ifstream ifstream(path.as_string().get_data());
		std::stringstream buffer;
		buffer << ifstream.rdbuf();
		std::string text = buffer.str();

		std::regex class_name_regex(R"(class\s+(\w+)\s*:\s*)");
		std::smatch match;
		std::string::const_iterator start(text.begin());
		while (std::regex_search(start, text.cend(), match, class_name_regex)) {
			assembly._types.add(type(path, match[1].str(), nullptr));
			start = match.suffix().first;
		}
	}

	void reflection::reflect_recurse(assembly& assembly, const path& path) {
		for (auto& i : std::filesystem::directory_iterator(path.as_string().get_data())) {
			if (std::filesystem::is_directory(i)) {
				reflect_recurse(assembly, uengine::path(uengine::string(i.path().string())));
			} else if (i.path().extension() == ".h") {
				reflect_header(assembly, uengine::path(uengine::string(i.path().string())));
			}
		}
	}

	assembly reflection::reflect(const path& directory) {
		assembly result;
		reflect_recurse(result, directory);
		return result;
	}
}