#pragma once

#include "vk_hanle.h"
#include "vk_config.h"
#include <vector>

namespace juce
{
class vk_device : vk_handle<VkDevice>
{
public:
	vk_device(
	    const VkInstance               instance,
	    const VkSurfaceFormatKHR       surface,
	    const std::vector<const char*> extension);
	~vk_device();
};
}        // namespace juce
