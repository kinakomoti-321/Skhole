#pragma once
#include <include.h>

#define SKHOLE_ERROR(error) std::cerr << "[ERROR] (" << __FILE__ << ":" << __LINE__ << "): " << error << std::endl;
#define SKHOLE_ASSERT(x) if(!(x)) { throw std::runtime_error("[Assert]" + std::string(#x) + " was not satisfied"); }
#define SKHOLE_UNIMPL(name) throw std::runtime_error("[Assert]" + std::string(name) + " is not implemented");

