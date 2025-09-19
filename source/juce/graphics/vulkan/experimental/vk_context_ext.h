#pragma once

#include <juce/graphics/vulkan/vk_context.h>
#include "vk_command_list.h"

namespace juce
{
class vk_context_ext : public vk_context
{
public:
	vk_context_ext(uint32_t cx, uint32_t cy, platform_handle handle);

	void init_command_list();

	vk_command_list m_command_lists[MAX_SYNC_FRAME]{};
};
}        // namespace juce
