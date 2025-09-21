#include "vk_context_ext.h"
#include "../vk_device.h"
#include "../vk_sync.h"
#include "../vk_swapchain.h"

namespace juce
{
vk_context_ext::vk_context_ext(uint32_t cx, uint32_t cy, platform_handle handle) :
    vk_context(cx, cy, handle)
{
	log_info("Use Extension Context 1.3");
	init_command_list();
}

void vk_context_ext::init_command_list()
{
	for(uint32_t i = 0; i < MAX_SYNC_FRAME; ++i) {
		VkCommandBuffer cmd = frames[i].m_cmd;
		m_command_lists[i].init(cmd);
	}
}

void vk_context_ext::begin_frame()
{
	VkFence fence = get_current_fence();
	vkWaitForFences(device(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device(), 1, &fence);

	VK(vkAcquireNextImageKHR(device(),
	                         swapchain(),
	                         UINT64_MAX, m_sync->get_image_available_semaphore(m_current_frame),
	                         VK_NULL_HANDLE, &m_swapchain_image_frame));

	VkImage swap_image = m_swapchain->get_images()[m_swapchain_image_frame];

	vk_command_list* cmd_list = get_current_command_list();

	vkResetCommandBuffer(cmd_list->handle(), 0);

	cmd_list->begin();

	cmd_list->resouce_barrier(image_transition{
	    swap_image,
	    {VK_IMAGE_ASPECT_COLOR_BIT},
	    resource_state::undefined,
	    resource_state::render_target});

	VkRenderingAttachmentInfo color_info{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
	color_info.imageView   = m_swapchain->get_image_views()[m_swapchain_image_frame];
	color_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	color_info.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_info.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
	color_info.clearValue  = {0.3, 0.2, 0.3, 1.f};

	VkRenderingInfo render_info{VK_STRUCTURE_TYPE_RENDERING_INFO};
	// render_info.renderArea.extent    = {width(), height()};
	render_info.renderArea.extent    = m_swapchain->extent();
	render_info.layerCount           = 1;
	render_info.colorAttachmentCount = 1;
	render_info.pColorAttachments    = &color_info;

	vkCmdBeginRendering(cmd_list->handle(), &render_info);
}

void vk_context_ext::end_frame()
{
	vk_command_list* cmd_list = get_current_command_list();

	vkCmdEndRendering(cmd_list->handle());

	cmd_list->resouce_barrier(image_transition(
	    m_swapchain->get_images()[m_swapchain_image_frame],
	    {},
	    resource_state::render_target,
	    resource_state::present));

	cmd_list->end();

	VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSemaphore wait_semaphore   = m_sync->get_image_available_semaphore(m_current_frame);
	VkSemaphore render_semaphore = m_sync->get_render_finished_semaphore(m_current_frame);

	VkSubmitInfo submit_info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit_info.pWaitDstStageMask    = &wait_stage_mask;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = cmd_list->handle_ptr();
	submit_info.waitSemaphoreCount   = 1;
	submit_info.pWaitSemaphores      = &wait_semaphore;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &render_semaphore;

	VK(vkQueueSubmit(m_device->graphics_queue(), 1, &submit_info, m_sync->get_inflight_fence(m_current_frame)));

	VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &render_semaphore;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = m_swapchain->handle_ptr();
	present_info.pImageIndices      = &m_swapchain_image_frame;

	VK(vkQueuePresentKHR(m_device->graphics_queue(), &present_info));

	m_current_frame = (m_current_frame + 1) % MAX_SYNC_FRAME;
}

vk_command_list* vk_context_ext::get_current_command_list()
{
	return &m_command_lists[m_current_frame];
}

}        // namespace juce
