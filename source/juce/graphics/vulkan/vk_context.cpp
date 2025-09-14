#include "vk_context.h"
#include "vk_config.h"
#include <juce/core/logger.h>
#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"

namespace juce
{
vk_context::vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle) :
    graphics_context(cx, cy, platform_handle)
{
	uint32_t version = VK_HEADER_VERSION_COMPLETE;

	log_info("Juce-Engine : Vulkan API ver %d.%d.%d\n", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

	// required instance extensions
	std::vector<const char*> required_extensions = {
#ifdef _WIN32
	    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__linux__)
	    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
	    VK_KHR_SURFACE_EXTENSION_NAME,
	    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

	std::vector<const char*> required_layers = {
#ifdef _DEBUG
	    "VK_LAYER_KHRONOS_validation"
#endif
	};

	m_instance = debug_new vk_instance(JUCE_ENGINE_NAME, required_extensions, required_layers);

	m_surface = debug_new vk_surface(m_instance->handle(), platform_handle);

	m_device = debug_new vk_device(m_instance->handle(), m_surface->handle());
}

vk_context::~vk_context()
{
	safe_delete(m_device);
	safe_delete(m_surface);
	safe_delete(m_instance);
}

void vk_context::on_resized(uint32 cx, uint32 cy)
{
	unused(cx);
	unused(cy);
}

}        // namespace juce
