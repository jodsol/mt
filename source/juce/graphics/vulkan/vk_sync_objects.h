#pragma once

#include <juce/core/typedef.h>
#include "vk_handle.h"
#include "vk_config.h"
#include <vector>

namespace juce
{
class vk_sync_objects
{
public:
	vk_sync_objects(VkDevice device);
	~vk_sync_objects();

	// 각 프레임에서 동기화 객체 접근
	VkSemaphore get_image_available_semaphore(uint32_t frame_index) const;
	VkSemaphore get_render_finished_semaphore(uint32_t frame_index) const;
	VkFence     get_inflight_fence(uint32_t frame_index) const;

	void wait_for_fence(uint32_t frame_index);
	void reset_fence(uint32_t frame_index);

private:
	VkDevice m_device;
	uint32_t m_max_frames_inflight = MAX_SYNC_FRAME;

	std::vector<VkSemaphore> m_image_available_semaphores;
	std::vector<VkSemaphore> m_render_finished_semaphores;
	std::vector<VkFence>     m_inflight_fences;

	void create_sync_objects();
	void destroy_sync_objects();
};
}        // namespace juce
