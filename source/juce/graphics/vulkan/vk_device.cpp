#include "vk_device.h"
#include "vk_context.h"
#define VMA_IMPLEMENTATION
// #define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
// #define VMA_STATIC_VULKAN_FUNCTIONS  0
#include "util/vk_mem_alloc.h"

namespace juce
{
vk_device::vk_device(vk_context* context) :
    m_context(context)
{
	create();
}

vk_device::~vk_device()
{
	release();
}

void vk_device::create_buffer(const buffer_create_info* info, vk_buffer_ext** pp_buffer)
{
}

void vk_device::create()
{
	init_vulkan_memory_allocator();

	m_device = m_context->get_logical_device_handle();

	VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	pool_info.queueFamilyIndex = m_context->graphics_queue_index();
	pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_pool));

	VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;
	info.commandPool        = m_pool;

	vkAllocateCommandBuffers(m_device, &info, &m_cmd);

	VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK(vkCreateFence(m_device, &fence_info, nullptr, &m_fence));

	// TODO Descriptor Pool ..etc
}

void vk_device::init_vulkan_memory_allocator()
{
	VmaAllocatorCreateInfo alloc_info{};
	alloc_info.device         = m_context->get_logical_device_handle();
	alloc_info.instance       = m_context->instance();
	alloc_info.physicalDevice = m_context->get_physical_device();
	// alloc_info.pVulkanFunctions = &funcs;
	alloc_info.vulkanApiVersion = VK_API_VERSION_1_3;
	VK(vmaCreateAllocator(&alloc_info, &m_allocator));
}

void vk_device::release()
{
	VK(vkDeviceWaitIdle(m_device));
	vmaDestroyAllocator(m_allocator);
	vkFreeCommandBuffers(m_device, m_pool, 1, &m_cmd);
	vk_safe_destroy(m_device, m_pool);
	vk_safe_destroy(m_device, m_fence);
}

}        // namespace juce
