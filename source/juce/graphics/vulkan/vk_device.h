#pragma once

#include <juce/core/typedef.h>
#include "vk_handle.h"
#include "vk_config.h"
#include <vector>
#include <optional>

namespace juce
{
// queue family result proxy
struct queue_indices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;
	std::optional<uint32_t> compute;
	std::optional<uint32_t> transfer;
};

struct vk_physical_device
{
	VkPhysicalDevice                 handle = VK_NULL_HANDLE;
	queue_indices                    queue_indices;
	VkQueueFlags                     flags{};        // flags of graphics family
	VkPhysicalDeviceMemoryProperties memory_props{};        // heap,type ..etc info
	VkPhysicalDeviceProperties       device_props{};        // gpu name, caps, driver info

	operator VkPhysicalDevice() const { return handle; }
};

class vk_device : public vk_handle<VkDevice>
{
public:
	vk_device(VkInstance instance, VkSurfaceKHR surface);
	~vk_device();

	void create_device(VkInstance instance, VkSurfaceKHR surface);

	bool choose_physical_device(VkInstance instance, VkSurfaceKHR surface,
	                            vk_physical_device* pp_gpu);

	void create_logical_device(VkSurfaceKHR surface);

	uint32_t graphics_queue_family_index() const;
	uint32_t present_queue_family_index() const;
	uint32_t compute_queue_family_index() const;
	uint32_t transfer_queue_family_index() const;

	const VkQueue graphics_queue() const;

	const VkQueue present_queue() const;

	const VkQueue compute_queue() const;

	const VkQueue transfer_queue() const;

	const VkSurfaceKHR surface() const;

	const vk_physical_device* get_gpu() const;

	static uint64_t get_gpu_available_vram_byte(const VkPhysicalDeviceMemoryProperties& mem);
	static bool     is_gpu_supported_surface(VkPhysicalDevice gpu, const VkSurfaceKHR surface);

	// protected:
	//  private:
	vk_physical_device m_gpu{};
	VkSurfaceKHR       m_surface        = VK_NULL_HANDLE;
	VkQueue            m_graphics_queue = VK_NULL_HANDLE;
	VkQueue            m_present_queue  = VK_NULL_HANDLE;
	VkQueue            m_compute_queue  = VK_NULL_HANDLE;
	VkQueue            m_transfer_queue = VK_NULL_HANDLE;
};
}        // namespace juce
