#pragma once

#include <juce/core/typedef.h>
#include "vk_handle.h"
#include "vk_config.h"
#include <vector>
#include <optional>

namespace juce
{

struct vk_physical_device
{
	VkPhysicalDevice       m_physical_device    = VK_NULL_HANDLE;
	int32_t                graphics_quque_index = -1;
	int32_t                transfer_queue_index = -1;
	std::optional<int32_t> graphics_index;

	operator VkPhysicalDevice()
	{
		return m_physical_device;
	}
};

class vk_device : vk_handle<VkDevice>
{
public:
	vk_device(
	    VkInstance                      instance,
	    VkSurfaceKHR                    surface,
	    const std::vector<const char*>& extension);
	~vk_device();

	VkPhysicalDevice choose_physical_device(
	    VkInstance                      instance,
	    VkSurfaceKHR                    surface,
	    const std::vector<const char*>& extension);

private:
	vk_physical_device m_gpu{};
};
}        // namespace juce
