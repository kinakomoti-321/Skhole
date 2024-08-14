#pragma once

// STD
#include <iostream>
#include <functional>
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>
#include <array>
#include <memory>
#include <filesystem>
#include <map>
#include <set>

#define NOMINMAX // avoid conflict with std::min, std::max and min, max macro in windows.h

// GLFW
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// Vulkan
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>


// VLG
#include <vector_like_glsl.h>

// Dear ImGui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>


