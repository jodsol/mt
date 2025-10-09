#pragma once

#include <juce/core/typedef.h>
#include "vk_config.h"

namespace juce
{
class vk_device
{
public:
	vk_device(vk_context* context);
	~vk_device();

	void create_buffer(const buffer_create_info* info, vk_buffer_ext** pp_buffer);
	void create_spv_from_file(const shader_create_info* info, vk_shader **pp_shader);


private:
	void create();
	void init_vulkan_memory_allocator();
	void release();

	vk_context*     m_context     = nullptr;
	VkDevice        m_device      = VK_NULL_HANDLE;
	uint32_t        m_queue_index = {};
	VkCommandPool   m_pool        = VK_NULL_HANDLE;
	VkCommandBuffer m_cmd         = VK_NULL_HANDLE;        // TODO : replace to vk_command_list;
	VkFence         m_fence       = VK_NULL_HANDLE;        // upload 전용 펜스
	VmaAllocator    m_allocator   = nullptr;
};

}        // namespace juce
