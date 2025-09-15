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

    uint32_t graphics_queue_family_index() const;
    uint32_t present_queue_family_index() const;
    uint32_t compute_queue_family_index() const;
    uint32_t transfer_queue_family_index() const;
	
	const VkQueue graphics_queue() const { return m_graphics_queue; }
	const VkQueue present_queue() const { return m_present_queue; }
	const VkQueue compute_queue() const { return m_compute_queue; }
	const VkQueue transfer_queue() const { return m_transfer_queue; }

	const VkSurfaceKHR get_surface() const
	{
		return m_surface;
	}

	const vk_physical_device* get_gpu() const
	{
		return &m_gpu;
	}

protected:
	static void print_vk_physical_device(const vk_physical_device& gpu);

private:
	vk_physical_device m_gpu{};
	VkSurfaceKHR       m_surface        = VK_NULL_HANDLE;
	VkQueue            m_graphics_queue = VK_NULL_HANDLE;
	VkQueue            m_present_queue  = VK_NULL_HANDLE;
	VkQueue            m_compute_queue  = VK_NULL_HANDLE;
	VkQueue            m_transfer_queue = VK_NULL_HANDLE;
};
}        // namespace juce
