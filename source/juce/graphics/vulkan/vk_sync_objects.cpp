#include "vk_sync_objects.h"

namespace juce
{
vk_sync_objects::vk_sync_objects(VkDevice device) :
    m_device(device)
{
	create_sync_objects();
}

vk_sync_objects::~vk_sync_objects()
{
	destroy_sync_objects();
}

VkSemaphore vk_sync_objects::get_image_available_semaphore(uint32_t frame_index) const
{
	return m_image_available_semaphores[frame_index];
}

VkSemaphore vk_sync_objects::get_render_finished_semaphore(uint32_t frame_index) const
{
	return m_render_finished_semaphores[frame_index];
}

VkFence vk_sync_objects::get_inflight_fence(uint32_t frame_index) const
{
	return m_inflight_fences[frame_index];
}

void vk_sync_objects::wait_for_fence(uint32_t frame_index)
{
	VkFence fence = get_inflight_fence(frame_index);
	vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT_MAX);
}

void vk_sync_objects::reset_fence(uint32_t frame_index)
{
	// VkFence fence = get_inflight_fence(frame_index);
	vkResetFences(m_device, 1, &m_inflight_fences[frame_index]);
}

void vk_sync_objects::create_sync_objects()
{
	m_image_available_semaphores.resize(m_max_frames_inflight);
	m_render_finished_semaphores.resize(m_max_frames_inflight);
	m_inflight_fences.resize(m_max_frames_inflight);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;        // 처음에는 signaled 상태

	for (size_t i = 0; i < m_max_frames_inflight; ++i) {
		VK(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphores[i]));
		VK(vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphores[i]));
		VK(vkCreateFence(m_device, &fence_info, nullptr, &m_inflight_fences[i]));
	}
}

void vk_sync_objects::destroy_sync_objects()
{
	for (size_t i = 0; i < m_max_frames_inflight; ++i) {
		vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
		vkDestroyFence(m_device, m_inflight_fences[i], nullptr);
	}
}
}        // namespace juce
