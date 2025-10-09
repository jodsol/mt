#pragma once

#include <juce/core/typedef.h>
#include <stdlib.h>
#include <memory>

namespace juce
{

struct vk_buffer
{
	VkBuffer       handle{};
	VkDeviceMemory memory{};
	VkDeviceSize   size{};
	VkDevice       device{};

	vk_buffer() = default;
	vk_buffer(VkDevice device) :
	    device(device) {};
	~vk_buffer();
};

struct vk_buffer_ext
{
	VkBuffer      handle{};
	VmaAllocation allocation{};
	VmaAllocator  allocator{};

	~vk_buffer_ext();
	void release();
};

}        // namespace juce
