#pragma once

#include "vk_config.h"
#include <juce/core/typedef.h>
#include <juce/graphics/context.h>
#include <memory>
#include <vector>

namespace juce
{
class vk_context : public graphics_context
{
public:
	vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle);
	~vk_context();
	void on_resized(uint32 cx, uint32 cy) override;

	vk_instance*     m_instance{nullptr};
	vk_surface*      m_surface{nullptr};
	vk_device*       m_device{nullptr};
	vk_swapchain*    m_swapchain{nullptr};
	vk_sync_objects* m_sync{nullptr};

	// VkCommandBuffer m_cmd[MAX_SYNC_FRAME];
	// VkCommandPool   m_cmd_pool[MAX_SYNC_FRAME];

	struct frame_object
	{
		VkCommandBuffer m_cmd;
		VkCommandPool   m_cmd_pool;
	};

	frame_object frames[MAX_SYNC_FRAME];
};
}        // namespace juce
