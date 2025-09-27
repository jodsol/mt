#pragma once

#include <juce/core/typedef.h>
#include <stdlib.h>
#include <memory>

// struct VmaAllocation_T;
typedef struct VmaAllocation_T* VmaAllocation;

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
	VmaAllocation allocation;
	VkDevice      device;

	static vk_buffer_ext* create_buffer_ext(VkDevice device, VkBuffer buf, VmaAllocation alloc);
};

}        // namespace juce
