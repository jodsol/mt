#pragma once

#if (_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	include <vulkan/vulkan.h>
#	include <vulkan/vulkan_win32.h>
#endif
#include <assert.h>
#include <string>

#define VK_DEFAULT_FENCE_TIMEOUT 100000000000

#define vk_load_instance_func(device, func)                             \
	do {                                                                \
		func = (PFN_##func) vkGetInstanceProcAddr(device, #func);       \
		assert(func && "Failed to load device-level function: " #func); \
	} while(0)

#define vk_load_device_func(device, func)                               \
	do {                                                                \
		func = (PFN_##func) vkGetDeviceProcAddr(device, #func);         \
		assert(func && "Failed to load device-level function: " #func); \
	} while(0)

#define VK(res)                                                                     \
	do {                                                                            \
		VkResult r = (res);                                                         \
		if(r != VK_SUCCESS) {                                                       \
			std::string msg = "file: " + std::string(__FILE__) +                    \
			    "\nline: " + std::to_string(__LINE__) +                             \
			    "\ncode: " + vk_error_to_str(r);                                    \
			MessageBoxA(nullptr, msg.c_str(), "Fatal Error", MB_OK | MB_ICONERROR); \
			exit(EXIT_FAILURE);                                                     \
		}                                                                           \
	} while(0)

// platform

#ifdef _WIN32
using win_handle = HWND;
#elif defined(__linux__)
using xcb_handle = xcb_window_t;
#elif defined(__APPLE__)
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

// define template
template <typename T>
struct VkDestroyFn;

// each funtion modify
#define reg_vk_destroy_fn(VkType, FuncName)              \
	template <>                                          \
	struct VkDestroyFn<VkType>                           \
	{                                                    \
		static void deleter(VkDevice device, VkType obj) \
		{                                                \
			FuncName(device, obj, nullptr);              \
		}                                                \
	}

// register function name
reg_vk_destroy_fn(VkFence, vkDestroyFence);
reg_vk_destroy_fn(VkBuffer, vkDestroyBuffer);
reg_vk_destroy_fn(VkShaderModule, vkDestroyShaderModule);
reg_vk_destroy_fn(VkPipeline, vkDestroyPipeline);
reg_vk_destroy_fn(VkPipelineLayout, vkDestroyPipelineLayout);
reg_vk_destroy_fn(VkDescriptorSetLayout, vkDestroyDescriptorSetLayout);
reg_vk_destroy_fn(VkSampler, vkDestroySampler);
reg_vk_destroy_fn(VkImage, vkDestroyImage);
reg_vk_destroy_fn(VkImageView, vkDestroyImageView);

template <typename T>
inline void __vk_safe_destroy(VkDevice device, T& handle)
{
	if(handle != VK_NULL_HANDLE) {
		VkDestroyFn<T>::deleter(device, handle);
		handle = VK_NULL_HANDLE;
	}
}

#define vk_safe_destroy(device, handle) __vk_safe_destroy(device, handle)

// vk function pointers
inline static PFN_vkCreateDebugUtilsMessengerEXT _vkCreateDebugUtilsMessengerEXT = nullptr;
#define vkCreateDebugUtilsMessengerEXT _vkCreateDebugUtilsMessengerEXT

inline const char* vk_error_to_str(VkResult result)
{
	switch(result) {
		case VK_SUCCESS:
			return "VK_SUCCESS";
		case VK_NOT_READY:
			return "VK_NOT_READY";
		case VK_TIMEOUT:
			return "VK_TIMEOUT";
		case VK_EVENT_SET:
			return "VK_EVENT_SET";
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET";
		case VK_INCOMPLETE:
			return "VK_INCOMPLETE";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED";
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL";
		case VK_ERROR_UNKNOWN:
			return "VK_ERROR_UNKNOWN";

		// Vulkan 1.1
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "VK_ERROR_OUT_OF_POOL_MEMORY";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE";

		// Vulkan 1.2
		case VK_ERROR_FRAGMENTATION:
			return "VK_ERROR_FRAGMENTATION";
		case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
			return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";

		// Vulkan 1.3
		case VK_PIPELINE_COMPILE_REQUIRED:
			return "VK_PIPELINE_COMPILE_REQUIRED";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "VK_ERROR_VALIDATION_FAILED_EXT";
		case VK_ERROR_INVALID_SHADER_NV:
			return "VK_ERROR_INVALID_SHADER_NV";
		case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
			return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
		case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
			return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
		case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
			return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		case VK_THREAD_IDLE_KHR:
			return "VK_THREAD_IDLE_KHR";
		case VK_THREAD_DONE_KHR:
			return "VK_THREAD_DONE_KHR";
		case VK_OPERATION_DEFERRED_KHR:
			return "VK_OPERATION_DEFERRED_KHR";
		case VK_OPERATION_NOT_DEFERRED_KHR:
			return "VK_OPERATION_NOT_DEFERRED_KHR";
		default:
			return "UNKNOWN_VK_RESULT";
	}
}

// inline VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_message_callback(
//     VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
//     VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback, void* p_user_data)
// {
// 	const char* severity = "";
// 	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
// 		severity = "VERBOSE";
// 	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
// 		severity = "INFO";
// 	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
// 		severity = "WARNING";
// 	else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
// 		severity = "ERROR";
// 	printf("[vulkan debug %s] : %s\n", severity, p_callback->pMessage);
// 	// char code[256]{};
// 	// sprintf(code, "[vulkan debug %s] : %s\n", severity, p_callback->pMessage);
// 	// OutputDebugString(code);

// 	return VK_FALSE;
// }

#include <iostream>

inline VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT /*message_type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback,
    void* /*p_user_data*/)
{
	const char* severity = "";
	const char* color    = "\033[0m";        // reset

	if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		severity = "VERBOSE";
		color    = "\033[36m";
	}
	else if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
		severity = "INFO";
		color    = "\033[32m";
	}
	else if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		severity = "WARNING";
		color    = "\033[33m";
	}
	else if(message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		severity = "ERROR";
		color    = "\033[31m";
	}

	std::cerr << color
	          << "================ Vulkan Debug ================\n"
	          << "Severity : " << severity << "\n"
	          << "Message  : " << p_callback->pMessage << "\n"
	          << "=============================================\n"
	          << "\033[0m";        // reset

	char buffer[2048];
	sprintf_s(buffer, "[vulkan debug %s] : %s\n", severity, p_callback->pMessage);
	OutputDebugStringA(buffer);

	return VK_FALSE;
}