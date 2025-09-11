#pragma once

#include "vk_config.h"
#include "vk_handle.h"
#include <vector>

namespace juce
{
class vk_surface : public vk_handle<VkSurfaceKHR>
{
public:
	vk_surface(VkInstance instance, void* plaform_handle);
	~vk_surface();

	bool valid() const;
	bool query(VkPhysicalDevice device);

private:
	VkInstance m_instance = VK_NULL_HANDLE;

	VkSurfaceCapabilities2KHR        caps;
	std::vector<VkSurfaceFormat2KHR> formats;
};
}        // namespace juce
