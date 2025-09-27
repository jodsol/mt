#include "vk_context_ext.h"
#include "../vk_device.h"
#include "../vk_sync.h"
#include "../vk_swapchain.h"
#include "vk_resouce_cache.h"

namespace juce
{
vk_context_ext::vk_context_ext(uint32_t cx, uint32_t cy, platform_handle handle) :
    vk_context(cx, cy, handle)
{
	log_info("Use Extension Context 1.3");
	init_command_list();
	init_render_targets();
	vk_resource_cache::initialize(this);
}

vk_context_ext::~vk_context_ext()
{
	vk_resource_cache::deinitialize();
}

void vk_context_ext::init_command_list()
{
	for(uint32_t i = 0; i < MAX_SYNC_FRAME; ++i) {
		VkCommandBuffer cmd = frames[i].m_cmd;
		m_command_lists[i].init(cmd);
	}
	if(m_device->graphics_queue_family_index() != m_device->transfer_queue_family_index()) {
		log_info("transfer command buffer use the transfer dedicated queue family index");
	}
	// m_transfer_pool.init(device(), m_device->transfer_queue_family_index());
	//  cmd = m_transfer_pool.allocate();
}

void vk_context_ext::init_render_targets()
{
	const VkImage* images      = m_swapchain->get_images().data();
	uint32_t       image_count = (uint32_t)m_swapchain->get_images().size();
	for(uint32_t i = 0; i < image_count; ++i) {
		vk_render_target& render_target = m_render_target[i] = {};

		render_target.extend      = m_swapchain->extent();
		render_target.layout      = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		render_target.view        = m_swapchain->get_image_view(i);
		render_target.image       = m_swapchain->get_image(i);
		render_target.clear_value = {{0.f, 0.f, 0.f, 1.f}};
		render_target.load_op     = load_operator::clear;
		render_target.store_op    = store_operator::store;
	}
}

void vk_context_ext::begin_frame()
{
	VkFence fence = get_current_fence();
	vkWaitForFences(device(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device(), 1, &fence);

	VK(vkAcquireNextImageKHR(device(), swapchain(), UINT64_MAX,
	                         m_sync->get_image_available_semaphore(m_current_frame), VK_NULL_HANDLE,
	                         &m_swapchain_image_frame));

	VkImage swap_image = m_swapchain->get_image(m_swapchain_image_frame);

	vk_command_list* cmd_list = get_current_command_list();

	cmd_list->reset();

	cmd_list->resouce_barrier(
	    image_transition{swap_image,
	                     {VK_IMAGE_ASPECT_COLOR_BIT},
	                     resource_state::undefined,
	                     resource_state::render_target});        // no excute at that time
}

void vk_context_ext::end_frame()
{
	vk_command_list* cmd_list = get_current_command_list();

	cmd_list->end_render_target();

	cmd_list->resouce_barrier(image_transition(m_swapchain->get_image(m_swapchain_image_frame), {},
	                                           resource_state::render_target,
	                                           resource_state::present));

	cmd_list->close();

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

	VK(vkQueueSubmit(m_device->graphics_queue(), 1, &submit_info,
	                 m_sync->get_inflight_fence(m_current_frame)));

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

vk_render_target* vk_context_ext::get_current_swapchain_render_target()
{
	return &m_render_target[m_swapchain_image_frame];
}

}        // namespace juce
