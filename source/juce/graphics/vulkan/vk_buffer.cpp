#include "vk_buffer.h"
#include "vk_config.h"

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

vk_buffer_ext* vk_buffer_ext::create_buffer_ext(VkDevice device, VkBuffer buf, VmaAllocation alloc)
{
	vk_buffer_ext* buffer = debug_new vk_buffer_ext;
	buffer->device        = device;
	buffer->handle        = buf;
	buffer->allocation    = alloc;
	return buffer;
}

}        // namespace juce
