#include "vk_context.h"
#include "vk_config.h"
#include <juce/core/logger.h>
#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_sync_objects.h"

namespace juce
{
vk_context::vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle) : graphics_context(cx, cy, platform_handle)
{
	uint32_t version = VK_HEADER_VERSION_COMPLETE;

	log_info("Juce-Engine : Vulkan API ver %d.%d.%d\n",
	         VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

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

	m_instance  = debug_new  vk_instance(ENGINE_NAME, required_extensions, required_layers);
	m_surface   = debug_new   vk_surface(m_instance->handle(), platform_handle);
	m_device    = debug_new    vk_device(m_instance->handle(), m_surface->handle());
	m_swapchain = debug_new vk_swapchain(m_device, cx, cy);
	m_sync      = debug_new      vk_sync_objects(m_device->handle());

	// Command Pool + Buffer 생성
	create_command_objects();
}

vk_context::~vk_context()
{
	destroy_command_objects();

	safe_delete(m_sync);
	safe_delete(m_swapchain);
	safe_delete(m_device);
	safe_delete(m_surface);
	safe_delete(m_instance);
}

void vk_context::on_resized(uint32 cx, uint32 cy)
{
	unused(cx);
	unused(cy);
}

void vk_context::create_command_objects()
{
	VkDevice device              = m_device->handle();
	uint32_t graphicsQueueFamily = m_device->graphics_queue_family_index();

	for (uint32_t i = 0; i < MAX_SYNC_FRAME; i++) {
		// Command Pool 생성
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsQueueFamily;

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &frames[i].m_cmd_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}

		// Command Buffer 할당
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool        = frames[i].m_cmd_pool;
		allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocInfo, &frames[i].m_cmd) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffer!");
		}
	}
}

void vk_context::destroy_command_objects()
{
	VkDevice device = m_device->handle();

	for (uint32_t i = 0; i < MAX_SYNC_FRAME; i++) {
		if (frames[i].m_cmd != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(device, frames[i].m_cmd_pool, 1, &frames[i].m_cmd);
			frames[i].m_cmd = VK_NULL_HANDLE;
		}

		if (frames[i].m_cmd_pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, frames[i].m_cmd_pool, nullptr);
			frames[i].m_cmd_pool = VK_NULL_HANDLE;
		}
	}
}
}        // namespace juce
