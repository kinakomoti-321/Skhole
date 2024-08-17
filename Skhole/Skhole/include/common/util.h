#pragma once

#include <include.h>

namespace Skhole {
	template <typename T>
	inline size_t GetVectorByteSize(const std::vector<T>& v) {
		return v.size() * sizeof(T);	
	}

	template <typename T>
	inline size_t GetTypeSize(const std::vector<T>& v) {
		return sizeof(T);	
	}
}
