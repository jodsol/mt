#include "vk_context.h"
#include "vk_config.h"
#include "vk_instnace.h"

namespace juce
{
vk_context::vk_context(uint32_t cx, uint32_t cy, void* platform_handle) :
    graphics_context(cx, cy, platform_handle)
{
	// required instance extensions
	std::vector<const char*> required_instance_extensions = {
#ifdef _WIN32
	    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__linux__)
	    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
	    VK_KHR_SURFACE_EXTENSION_NAME,
	    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

	m_instance = std::unique_ptr<vk_instance>(
	    new vk_instance(juce_engine_name,
	                    required_instance_extensions,
	                    true));
}

void vk_context::on_resized(uint32 cx, uint32 cy)
{
	unused(cx);
	unused(cy);
}

VkInstance vk_context::get_instance() const
{
	if (m_instance)
		return m_instance->get_handle();
	return nullptr;
}

}        // namespace juce
