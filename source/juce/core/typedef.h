#pragma once

// newmeric
#include <stdint.h>
typedef uint8_t  byte, uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#ifdef _WIN32
#	include <windows.h>
using win_handle = HWND;
#elif defined(__linux__)
#	include <xcb/xcb.h>
using xcb_handle = xcb_window_t;
#elif defined(__APPLE__)
#	include <objc/objc.h>
using mac_handle = void*;        // MoltenVK에서는 NSView*나 CAMetalLayer*를 사용
#endif

using platform_handle =
#ifdef _WIN32
    win_handle;
#elif defined(__linux__)
    xcb_handle;
#elif defined(__APPLE__)
    mac_handle;
#endif

// Win32
struct HWND__;
typedef HWND__* HWND;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

// Juce Forward Declare
namespace juce
{
// frontend
class graphics_context;

// backend
// vulkan
class vk_instance;
class vk_surface;
class vk_device;
class vk_swapchain;
class vk_sync_objects;
}        // namespace juce

#include "defines.h"