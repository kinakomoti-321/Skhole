#pragma once
#include <include.h>

namespace Skhole {

	inline bool GetFileExtension(const std::string& filepath, std::string& extension) {
		size_t dotIndex = filepath.find_last_of(".");
		if (dotIndex == std::string::npos) {
			return false;
		}
		extension = filepath.substr(dotIndex + 1);
		return true;
	}

	inline bool GetFileName(const std::string& filepath, std::string& filename) {
		size_t dotIndex = filepath.find_last_of("/");
		if (dotIndex == std::string::npos) {
			return false;
		}

		filename = filepath.substr(dotIndex + 1);
		return true;
	}
}

