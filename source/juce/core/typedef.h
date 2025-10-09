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

// Vulkan
typedef struct VkBuffer_T*       VkBuffer;
typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef struct VkDevice_T*       VkDevice;
typedef uint64_t                 VkDeviceSize;

// VMA
typedef struct VmaAllocator_T*  VmaAllocator;
typedef struct VmaAllocation_T* VmaAllocation;

// Juce Forward Declare
namespace juce
{
// frontend
class context;
class scene;

// backend

// vulkan
class vk_context;
class vk_instance;
class vk_surface;
class vk_logical_device;
class vk_swapchain;
class vk_sync;
class vk_command_list;
class vk_device;

// struct
struct vk_shader;
struct vk_buffer;
struct vk_buffer_ext;
struct vk_render_target;

// experimental
class vk_context_ext;

// engine enum
enum class render_target_type {
	color,
	depth,
	stencil,
	depth_stencil
};

enum class load_operator {
	load,
	clear,
	discard
};

enum class store_operator {
	store,
	discard,
	no_access
};

enum class resource_layout {
	undefined,
	render_target,
	present
};

enum class buffer_type {
	vertex,
	index,
	uniform
};

struct buffer_create_info
{
	buffer_type type;
	const void* data;
	uint32_t    size;
};

struct clear_value
{
	float color[4];
};

// owned: vk_logical_device에서 생성된 자원일 때
// imported: pure heap에서 만들었을 때
enum class alloc_scope {
	owned,
	imported
};

enum class shader_stage {
	vertex,
	pixel,
	geometry,
	hull
};

struct shader_create_info {
  shader_stage stage;
  const char* filename;
  const char* entry;
  // const char* include;
  // const char* defines;
};

}        // namespace juce

#include "defines.h"