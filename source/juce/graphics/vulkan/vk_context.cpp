#include "vk_context.h"
#include "vk_config.h"
#include <juce/core/logger.h>
#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_logical_device.h"
#include "vk_swapchain.h"
#include "vk_sync.h"
#include "util/vk_debug.h"
#include "vk_command_list.h"
#include "vk_render_target.h"
#include "vk_device.h"

namespace juce
{
vk_context::vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle) :
    context(cx, cy, platform_handle)
{
	uint32_t version = VK_HEADER_VERSION_COMPLETE;

	log_info("Juce-Engine : Vulkan API ver %d.%d.%d\n", VK_VERSION_MAJOR(version),
	         VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

	std::vector<const char*> required_extensions = {
#ifdef _WIN32
	    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__linux__)
	    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
	    VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

	std::vector<const char*> required_layers = {
#ifdef _DEBUG
	    "VK_LAYER_KHRONOS_validation"
#endif
	};

	m_instance       = debug_new       vk_instance(ENGINE_NAME, required_extensions, required_layers);
	m_surface        = debug_new        vk_surface(m_instance->handle(), platform_handle);
	m_logical_device = debug_new vk_logical_device(m_instance->handle(), m_surface->handle());
	m_swapchain      = debug_new      vk_swapchain(m_logical_device, cx, cy);
	m_sync           = debug_new           vk_sync(m_logical_device->handle());

	vk_debug::print_queue_families(m_logical_device);
	// Command Pool + Buffer 생성
	create_command_objects();

	m_device = debug_new vk_device(this);
}

vk_context::~vk_context()
{
	// 중요 모는 gpu 작업이 끝날떄까지 기다림
	vkDeviceWaitIdle(m_logical_device->handle());
	safe_delete(m_device);

	destroy_command_objects();

	safe_delete(m_sync);
	safe_delete(m_swapchain);
	safe_delete(m_logical_device);
	safe_delete(m_surface);
	safe_delete(m_instance);
}

void vk_context::resize_frame(uint32 cx, uint32 cy)
{
	log_debug("resize frame : %d %d", cx, cy);
	if(width() != cx || height() != cy) {
		log_debug("recreate swapcahin : %d %d", cx, cy);
		m_swapchain->recreate_swapchain(cx, cy);
	}
	set_size(cx, cy);
}

void vk_context::begin_frame()
{
	VkDevice device = get_logical_device_handle();

	VkFence fence = get_current_fence();
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &fence);

	VK(vkAcquireNextImageKHR(device, swapchain(), UINT64_MAX,
	                         m_sync->get_image_available_semaphore(m_current_frame), VK_NULL_HANDLE,
	                         &m_swapchain_image_frame));

	vk_command_list* cmd_list = get_current_command_list();

	cmd_list->reset();

	vk_render_target* rtv = get_current_swapchain_render_target();

	cmd_list->resouce_barrier(
	    image_transition{rtv->image,
	                     {VK_IMAGE_ASPECT_COLOR_BIT},
	                     resource_layout::undefined,
	                     resource_layout::render_target});        // no excute at that time

	cmd_list->begin_render_target(1, &rtv, nullptr);
}

void vk_context::end_frame()
{
	vk_command_list*  cmd_list = get_current_command_list();
	vk_render_target* rtv      = m_swapchain->get_render_target(m_swapchain_image_frame);

	cmd_list->end_render_target();

	cmd_list->resouce_barrier(image_transition(rtv->image, {VK_IMAGE_ASPECT_COLOR_BIT},
	                                           resource_layout::render_target,
	                                           resource_layout::present));

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

	VK(vkQueueSubmit(m_logical_device->graphics_queue(), 1, &submit_info,
	                 m_sync->get_inflight_fence(m_current_frame)));

	VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &render_semaphore;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = m_swapchain->handle_ptr();
	present_info.pImageIndices      = &m_swapchain_image_frame;

	VK(vkQueuePresentKHR(m_logical_device->graphics_queue(), &present_info));

	m_current_frame = (m_current_frame + 1) % MAX_SYNC_FRAME;
}

vk_command_list* vk_context::get_current_command_list()
{
	return m_frames[m_current_frame].cmd;
}

uint32_t vk_context::current_frame() const
{
	return m_current_frame;
}

uint32_t vk_context::swapchain_frame() const
{
	return m_swapchain_image_frame;
}

VkInstance vk_context::instance() const
{
	return m_instance->handle();
}

VkSurfaceKHR vk_context::surface() const
{
	return m_surface->handle();
}

VkDevice vk_context::get_logical_device_handle() const
{
	return m_logical_device->handle();
}

VkPhysicalDevice vk_context::get_physical_device() const
{
	return m_logical_device->get_gpu()->handle;
}

VkSwapchainKHR vk_context::swapchain() const
{
	return m_swapchain->handle();
}

VkFence vk_context::get_current_fence()
{
	return m_sync->get_inflight_fence(m_current_frame);
}

vk_render_target* vk_context::get_current_swapchain_render_target()
{
	return m_swapchain->get_render_target(m_swapchain_image_frame);
}

vk_device* vk_context::get_graphics_device() const
{
	return m_device;
}

VkQueue vk_context::graphics_queue() const
{
	return m_logical_device->graphics_queue();
}

VkQueue vk_context::transfer_queue() const
{
	return m_logical_device->transfer_queue();
}

uint32_t vk_context::graphics_queue_index() const
{
	return m_logical_device->graphics_queue_family_index();
}

uint32_t vk_context::transfer_queue_index() const
{
	return m_logical_device->transfer_queue_family_index();
}

void vk_context::create_command_objects()
{
	VkDevice device                = m_logical_device->handle();
	uint32_t graphics_queue_family = m_logical_device->graphics_queue_family_index();

	for(uint32_t i = 0; i < MAX_SYNC_FRAME; i++) {
		// Command Pool 생성
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = graphics_queue_family;

		VK(vkCreateCommandPool(device, &pool_info, nullptr, &m_frames[i].cmd_pool));

		m_frames[i].cmd = debug_new vk_command_list(m_logical_device->handle(), m_frames[i].cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}
}

void vk_context::destroy_command_objects()
{
	VkDevice device = m_logical_device->handle();

	for(uint32_t i = 0; i < MAX_SYNC_FRAME; i++) {
		safe_delete(m_frames[i].cmd);

		if(m_frames[i].cmd_pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, m_frames[i].cmd_pool, nullptr);
			m_frames[i].cmd_pool = VK_NULL_HANDLE;
		}
	}
}

}        // namespace juce
