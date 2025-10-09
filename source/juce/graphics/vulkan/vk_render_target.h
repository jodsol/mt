#pragma once

#include <juce/core/typedef.h>
#include <array>
#include "vk_config.h"

namespace juce
{

struct vk_render_target
{
	render_target_type type;
	VkDevice           device;
	VkImage            image;
	VkDeviceMemory     mem;
	VkImageView        view;
	VkFormat           format;
	VkExtent2D         extend{};
	VkImageLayout      layout;
	VkClearValue       clear_value;

	// extension
	load_operator  load_op;
	store_operator store_op;

	// clear_value clear_value;

	union
	{
		float   clear_depth;
		uint8_t clear_stencil;
	};

	alloc_scope scope = alloc_scope::owned;

	~vk_render_target()
	{
		release();
	};

	void release()
	{
		if(scope == alloc_scope::owned) {
			vk_safe_destroy(device, image);
			if(mem) {
				vkFreeMemory(device, mem, nullptr);
				mem = VK_NULL_HANDLE;
			}
		}
		vk_safe_destroy(device, view);
	};
};

}        // namespace juce
