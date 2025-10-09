#include "vk_buffer.h"
#include "vk_config.h"
#include "util/vk_mem_alloc.h"

namespace juce
{
vk_buffer::~vk_buffer()
{
	if(handle) {
		vkDestroyBuffer(device, handle, nullptr);
		handle = VK_NULL_HANDLE;
	}
	if(memory) {
		vkFreeMemory(device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
	// vk_safe_destroy(device, handle);
}

vk_buffer_ext::~vk_buffer_ext()
{
	release();
}

void vk_buffer_ext::release()
{
	if(handle) {
		vmaDestroyBuffer(allocator, handle, allocation);
		handle     = VK_NULL_HANDLE;
		allocation = VK_NULL_HANDLE;
	}
}

}        // namespace juce
