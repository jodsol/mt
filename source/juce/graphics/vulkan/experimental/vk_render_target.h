#pragma once

#include <juce/core/typedef.h>
#include "../vk_config.h"

namespace juce
{

struct vk_render_target
{
	render_target_type type;
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

	// union
	float clear_color[4];
	
	union {
        float   clear_depth;
        uint8_t clear_stencil;
    };
};

}        // namespace juce
