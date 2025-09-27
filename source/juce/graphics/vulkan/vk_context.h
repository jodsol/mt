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
		VkCommandBuffer m_cmd{VK_NULL_HANDLE};
		VkCommandPool   m_cmd_pool{VK_NULL_HANDLE};
	};

	frame_object frames[MAX_SYNC_FRAME];

	// get indexed frame
	uint32_t current_frame() const override;
	uint32_t swapchain_frame() const override;

	// get raw vulkan handles
	VkInstance     instance() const;
	VkSurfaceKHR   surface() const;
	VkDevice       device() const;
	VkSwapchainKHR swapchain() const;
	VkQueue        graphics_queue() const;
	VkQueue        transfer_queue() const;
	uint32_t       graphics_queue_index() const;
	uint32_t       transfer_queue_index() const;

	VkFence get_current_fence();

	// ext class 에서 private 맴버도 허용
	friend class vk_context_ext;

	// private:
	uint32_t m_frame_number          = 0;
	uint32_t m_current_frame         = 0;
	uint32_t m_swapchain_image_frame = 0;

	// 래퍼 class 들을 동적으로 만들 필요가 있나 싶음
	// init 을 생성에 쓰고 release 함수를 해제하는거 검토
	vk_instance*  m_instance{nullptr};
	vk_surface*   m_surface{nullptr};
	vk_device*    m_device{nullptr};
	vk_swapchain* m_swapchain{nullptr};
	vk_sync*      m_sync{nullptr};

	void                     create_command_objects();
	void                     destroy_command_objects();
	VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags);
	void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout,
	                      VkImageLayout newLayout);
};
}        // namespace juce
