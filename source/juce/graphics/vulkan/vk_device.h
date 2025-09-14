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

	operator VkPhysicalDevice() const
	{
		return handle;
	}
};

class vk_device : public vk_handle<VkDevice>
{
public:
	vk_device(VkInstance instance, VkSurfaceKHR surface);
	~vk_device();

	void create_device(VkInstance instance, VkSurfaceKHR surface);

	bool choose_physical_device(VkInstance instance, VkSurfaceKHR surface, vk_physical_device* pp_gpu);

	void create_logical_device(VkSurfaceKHR surface);

	void print_vk_logical_device();

	VkSurfaceKHR get_surface() {
		return m_surface;
	}

protected:
	static void print_vk_physical_device(const vk_physical_device& gpu);

private:
	vk_physical_device m_gpu{};
	VkSurfaceKHR       m_surface = VK_NULL_HANDLE;
	VkQueue            m_graphics_queue = VK_NULL_HANDLE;
	VkQueue            m_present_queue  = VK_NULL_HANDLE;
	VkQueue            m_compute_queue  = VK_NULL_HANDLE;
	VkQueue            m_transfer_queue = VK_NULL_HANDLE;
};
}        // namespace juce
