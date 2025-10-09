#pragma once

#include "vk_config.h"
#include <juce/core/typedef.h>
#include <juce/graphics/context.h>
#include <memory>
#include <vector>

namespace juce
{
class vk_context : public context
{
public:
	vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle);
	~vk_context();

	void resize_frame(uint32 cx, uint32 cy) override;

	void begin_frame() override;
	void end_frame() override;

	struct frame_object
	{
		VkCommandPool    cmd_pool{VK_NULL_HANDLE};
		vk_command_list* cmd = nullptr;
	};

	frame_object     m_frames[MAX_SYNC_FRAME];
	vk_command_list* get_current_command_list();

	// get indexed frame
	uint32_t current_frame() const override;
	uint32_t swapchain_frame() const override;

	// get raw vulkan handles
	VkInstance       instance() const;
	VkSurfaceKHR     surface() const;
	VkDevice         get_logical_device_handle() const;
	VkPhysicalDevice get_physical_device() const;
	VkSwapchainKHR   swapchain() const;
	VkQueue          graphics_queue() const;
	VkQueue          transfer_queue() const;
	uint32_t         graphics_queue_index() const;
	uint32_t         transfer_queue_index() const;

	VkFence           get_current_fence();
	vk_render_target* get_current_swapchain_render_target();

	vk_device* get_graphics_device() const;

	// private:
	uint32_t m_frame_number          = 0;
	uint32_t m_current_frame         = 0;
	uint32_t m_swapchain_image_frame = 0;

	vk_instance*       m_instance{nullptr};
	vk_surface*        m_surface{nullptr};
	vk_logical_device* m_logical_device{nullptr};
	vk_swapchain*      m_swapchain{nullptr};
	vk_sync*           m_sync{nullptr};
	vk_device*         m_device;

	void create_command_objects();
	void destroy_command_objects();
};
}        // namespace juce
