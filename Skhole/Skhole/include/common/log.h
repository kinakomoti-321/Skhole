#pragma once
#include <include.h>

#define SKHOLE_ERROR(error) std::cerr << "[ERROR] (" << __FILE__ << ":" << __LINE__ << "): " << error << std::endl;
#define SKHOLE_WARN(warn) std::cerr << "[WARN] (" << __FILE__ << ":" << __LINE__ << "): " << warn << std::endl;
#define SKHOLE_ASSERT(x) if(!(x)) { throw std::runtime_error("[Assert]" + std::string(#x) + " was not satisfied"); }
#define SKHOLE_ABORT(error) throw std::runtime_error("[Abort]" + std::string(error));
#define SKHOLE_UNIMPL(name) throw std::runtime_error("[Assert]" + std::string(name) + " is not implemented");
#define SKHOLE_LOG(message) std::cout << "[LOG] " << message << std::endl;

#define SKHOLE_LOG_SECTION(section) std::cout << "---------------------------------" << std::endl << " " << std::string(section) << std::endl << "---------------------------------" << std::endl;
