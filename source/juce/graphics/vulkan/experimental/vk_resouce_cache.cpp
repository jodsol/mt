#include "vk_resouce_cache.h"
#include "vk_context_ext.h"
#include "../vk_device.h"
#include "../vk_buffer.h"

namespace juce
{
void vk_resource_cache::initialize(vk_context* context)
{
	if(m_is_init)
		return;

	m_context      = context;
	m_device       = context->device();
	m_gpu_mem_prop = context->m_device->get_gpu()->memory_props;

	uint32_t transfer_queue_index = m_context->graphics_queue_index();
	m_queue                       = m_context->graphics_queue();
	// if(m_context->graphics_queue() != m_context->transfer_queue()) {
	// 	log_debug("choose sub resource system transfer queue");
	// 	transfer_queue_index = m_context->transfer_queue_index();
	// 	m_queue              = m_context->transfer_queue();
	// }

	VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	pool_info.queueFamilyIndex = transfer_queue_index;
	pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool));

	VkCommandBufferAllocateInfo cmd_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	cmd_info.commandBufferCount = 1;
	cmd_info.commandPool        = m_command_pool;
	cmd_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK(vkAllocateCommandBuffers(m_device, &cmd_info, &m_cmd));

	VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK(vkCreateFence(m_device, &fence_info, nullptr, &m_fence));

	m_is_init = true;
}

void vk_resource_cache::deinitialize()
{
	if(!m_is_init)
		return;
	vkDeviceWaitIdle(m_device);
	vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);

	vk_safe_destroy(m_device, m_fence);
	vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_cmd);
	vk_safe_destroy(m_device, m_command_pool);
	m_is_init = false;
}

void vk_resource_cache::create_buffer(const buffer_create_info* info, vk_buffer** pp_buffer)
{
	vk_buffer staging_buffer{m_device};

	commit_resource_buffer(info->cb_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                       staging_buffer.handle, staging_buffer.memory);

	void* mapped = nullptr;
	VK(vkMapMemory(m_device, staging_buffer.memory, 0, info->cb_size, 0, &mapped));
	memcpy(mapped, info->p_data, info->cb_size);
	vkUnmapMemory(m_device, staging_buffer.memory);

	VkBufferUsageFlags dst_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	switch(info->type) {
		case buffer_type::vertex:
			dst_flags = dst_flags | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		case buffer_type::index:
			dst_flags = dst_flags | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		case buffer_type::uniform:
			dst_flags = dst_flags | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			break;
	}

	vk_buffer* gpu_buffer = debug_new vk_buffer(m_device);

	commit_resource_buffer(info->cb_size, dst_flags,
	                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                       gpu_buffer->handle, gpu_buffer->memory);

	VK(vkResetFences(m_device, 1, &m_fence));
	VK(vkResetCommandBuffer(m_cmd, 0));

	VkCommandBufferBeginInfo cmd_begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(m_cmd, &cmd_begin_info);

	VkBufferCopy copy_region{0, 0, info->cb_size};
	vkCmdCopyBuffer(m_cmd, staging_buffer.handle, gpu_buffer->handle, 1, &copy_region);

	vkEndCommandBuffer(m_cmd);

	// excute and wait
	VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &m_cmd;

	vkQueueSubmit(m_queue, 1, &submit_info, m_fence);

	vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX);

	*pp_buffer = gpu_buffer;
	gpu_buffer = nullptr;
}

void vk_resource_cache::commit_resource_buffer(
    VkDeviceSize          size,
    VkBufferUsageFlags    usage,
    VkMemoryPropertyFlags mem_flags,
    VkBuffer&             p_buffer,
    VkDeviceMemory&       p_memory)
{
	VkBuffer       buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;

	VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	info.size        = size;
	info.usage       = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK(vkCreateBuffer(m_device, &info, nullptr, &buffer));

	VkMemoryRequirements mem_req{};
	vkGetBufferMemoryRequirements(m_device, buffer, &mem_req);

	VkMemoryAllocateInfo alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	alloc_info.allocationSize  = mem_req.size;
	alloc_info.memoryTypeIndex = get_memory_type_index(mem_req.memoryTypeBits, mem_flags);

	VK(vkAllocateMemory(m_device, &alloc_info, nullptr, &memory));
	VK(vkBindBufferMemory(m_device, buffer, memory, 0));

	p_buffer = buffer;
	p_memory = memory;
}

uint32_t vk_resource_cache::get_memory_type_index(uint32_t type_bit, VkMemoryPropertyFlags props)
{
	assert(m_is_init && "vk is not initialized");
	for(uint32_t i = 0; i < m_gpu_mem_prop.memoryTypeCount; i++) {
		if((type_bit & 1) == 1) {
			if((m_gpu_mem_prop.memoryTypes[i].propertyFlags & props) == props) {
				return i;
			}
		}
		type_bit >>= 1;
	}
	throw "could not find a suitable memory type";
}

}        // namespace juce
