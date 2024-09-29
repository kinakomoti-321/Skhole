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

	inline std::string DeleteFileExtension(const std::string file) {
		size_t dotIndex = file.find_last_of(".");
		if (dotIndex == std::string::npos) {
			return file;
		}
		return file.substr(0, dotIndex);
	}

	inline void SeparatePathAndFile(const std::string& filepath, std::string& path, std::string& filename) {
		std::filesystem::path pathSystem(filepath);
		path = pathSystem.parent_path().string();
		filename = pathSystem.filename().string();
	}

	inline bool GetFileName(const std::string& filepath, std::string& filename) {
		size_t dotIndex = filepath.find_last_of("/");
		if (dotIndex == std::string::npos) {
			return false;
		}

		filename = filepath.substr(dotIndex + 1);
		return true;
	}

	inline std::string GethCurrentTimeString() {
		std::time_t now = std::time(nullptr);
		std::tm* localTime = std::localtime(&now);

		// 時刻をフォーマットする（例: YYYYMMDD_HHMMSS）
		std::ostringstream oss;
		oss << std::put_time(localTime, "%Y%m%d_%H%M%S");

		return oss.str();
	}

	inline std::string NumbertToSerial(int number, int paddingDigit) {
		std::ostringstream os;
		os << std::setw(paddingDigit) << std::setfill('0') << number;
		return os.str();
	}

}

