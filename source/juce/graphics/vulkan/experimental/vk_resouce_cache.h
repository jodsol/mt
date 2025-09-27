#pragma once

#include <juce/core/typedef.h>
#include <juce/graphics/vulkan/vk_config.h>

namespace juce
{
class vk_resource_cache
{
public:
	vk_resource_cache() = default;

	static void initialize(vk_context* context);
	static void deinitialize();

	static void create_buffer(const buffer_create_info* info,
	                          vk_buffer**               pp_buffer);

	static void commit_resource_buffer(VkDeviceSize          size,
	                                   VkBufferUsageFlags    usage,
	                                   VkMemoryPropertyFlags mem_flags,
	                                   VkBuffer&             buffer,
	                                   VkDeviceMemory&       memory);

	static uint32_t get_memory_type_index(uint32_t              type_bit,
	                                      VkMemoryPropertyFlags flags);

	inline static vk_context*                      m_context = nullptr;
	inline static VkDevice                         m_device  = VK_NULL_HANDLE;
	inline static VkPhysicalDeviceMemoryProperties m_gpu_mem_prop;
	// 리소스 전용 커맨드 풀
	inline static VkCommandPool   m_command_pool = VK_NULL_HANDLE;
	inline static VkCommandBuffer m_cmd          = VK_NULL_HANDLE;
	inline static VkQueue         m_queue        = VK_NULL_HANDLE;
	inline static VkFence         m_fence        = VK_NULL_HANDLE;

private:
	inline static bool m_is_init = false;
};

}        // namespace juce
