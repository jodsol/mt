#pragma once

#include <juce/graphics/vulkan/vk_context.h>
#include "vk_command_list.h"
#include "vk_render_target.h"

namespace juce
{
class vk_context_ext : public vk_context
{
public:
	vk_context_ext(uint32_t cx, uint32_t cy, platform_handle handle);
	virtual ~vk_context_ext();
	void init_command_list();
	void init_render_targets();

	void begin_frame() final;

	void end_frame() final;

	vk_command_list* get_current_command_list();

	vk_command_list m_command_lists[MAX_SYNC_FRAME]{};

	vk_render_target* get_current_swapchain_render_target();
	vk_render_target  m_render_target[8];

	VkCommandBuffer cmd;
	vk_command_list m_transfer_cmd;
};
}        // namespace juce
