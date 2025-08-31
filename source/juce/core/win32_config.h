#pragma once

// window
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // std::numeric_limits<uint32_t>::max() 치환 해결
#include <assert.h>
#include <windows.h>

// vulkan
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>