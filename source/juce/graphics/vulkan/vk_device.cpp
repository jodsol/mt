#include "vk_device.h"

namespace juce
{
vk_device::vk_device(
    const VkInstance               instance,
    const VkSurfaceFormatKHR       surface,
    const std::vector<const char*> extension)
{
}

vk_device::~vk_device()
{
	if (m_handle)
		vkDestroyDevice(m_handle, nullptr);
}

}        // namespace juce
