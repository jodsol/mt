#include "vk_context.h"
#include "vk_config.h"
#include <juce/core/logger.h>
#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_sync.h"

namespace juce
{
vk_context::vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle) :
    context(cx, cy, platform_handle)
{
	uint32_t version = VK_HEADER_VERSION_COMPLETE;

	log_info("Juce-Engine : Vulkan API ver %d.%d.%d\n",
	         VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

	std::vector<const char*> required_extensions = {
#ifdef _WIN32
	    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__linux__)
	    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
	    VK_KHR_SURFACE_EXTENSION_NAME,
	    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

	std::vector<const char*> required_layers = {
#ifdef _DEBUG
	    "VK_LAYER_KHRONOS_validation"
#endif
	};

	m_instance  = debug_new  vk_instance(ENGINE_NAME, required_extensions, required_layers);
	m_surface   = debug_new   vk_surface(m_instance->handle(), platform_handle);
	m_device    = debug_new    vk_device(m_instance->handle(), m_surface->handle());
	m_swapchain = debug_new vk_swapchain(m_device, cx, cy);
	m_sync      = debug_new      vk_sync(m_device->handle());

	// Command Pool + Buffer 생성
	create_command_objects();
}

vk_context::~vk_context()
{
	// 중요 모는 gpu 작업이 끝날떄까지 기다림
	vkDeviceWaitIdle(m_device->handle());

	destroy_command_objects();

	safe_delete(m_sync);
	safe_delete(m_swapchain);
	safe_delete(m_device);
	safe_delete(m_surface);
	safe_delete(m_instance);
}

void vk_context::on_resized(uint32 cx, uint32 cy)
{
	unused(cx);
	unused(cy);
}

// Command Buffer reset → vkBeginCommandBuffer → 이미지 레이아웃 전환 & clear → vkEndCommandBuffer → vkQueueSubmit → vkQueuePresentKHR
void vk_context::draw_frame(float dt)
{
	frame_object& frame = frames[m_current_frame];

	// --- draw_1: GPU가 이전 프레임 끝날 때까지 대기
	VkFence fence = m_sync->get_inflight_fence(m_current_frame);
	vkWaitForFences(m_device->handle(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_device->handle(), 1, &fence);

	// --- draw_2: Swapchain 이미지 요청
	uint32_t image_index;
	VK(vkAcquireNextImageKHR(
	    m_device->handle(),
	    m_swapchain->handle(),
	    UINT64_MAX,
	    m_sync->get_image_available_semaphore(m_current_frame),
	    VK_NULL_HANDLE,
	    &image_index));

	m_swapchain_image_frame = image_index;

	// --- draw_3: Command Buffer reset & begin
	VkCommandBuffer cmd = frame.m_cmd;
	vkResetCommandBuffer(cmd, 0);

	VkCommandBufferBeginInfo begin_info = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK(vkBeginCommandBuffer(cmd, &begin_info));

	// --- draw_4: 이미지 레이아웃 전환 + clear + 다시 present용으로 전환
	VkImage swap_image = m_swapchain->get_images()[image_index];
	transition_image(cmd, swap_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	VkClearColorValue clear_value;
	// float             time  = fmod(m_frame_number * dt, 2.0f);
	static float time  = 0.0f;
	float        speed = 0.3f;
	time += dt;
	float flash = 1.f - std::fabsf(std::sin(3.14159265f * speed * time));
	clear_value = {{flash, flash, flash, 1.0f}};

	VkImageSubresourceRange clear_range{};
	clear_range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	clear_range.baseMipLevel   = 0;
	clear_range.levelCount     = 1;
	clear_range.baseArrayLayer = 0;
	clear_range.layerCount     = 1;

	vkCmdClearColorImage(cmd, swap_image, VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &clear_range);

	transition_image(cmd, swap_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	VK(vkEndCommandBuffer(cmd));

	// --- draw_5: Command Buffer 제출
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore          wait_semaphore   = m_sync->get_image_available_semaphore(m_current_frame);
	VkSemaphore          signal_semaphore = m_sync->get_render_finished_semaphore(m_current_frame);
	VkPipelineStageFlags wait_stage       = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit_info.waitSemaphoreCount   = 1;
	submit_info.pWaitSemaphores      = &wait_semaphore;
	submit_info.pWaitDstStageMask    = &wait_stage;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &cmd;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &signal_semaphore;

	VK(vkQueueSubmit(m_device->graphics_queue(), 1, &submit_info, fence));

	// --- draw_6: Present
	VkPresentInfoKHR present_info{};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &signal_semaphore;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = m_swapchain->handle_ptr();
	present_info.pImageIndices      = &image_index;

	VK(vkQueuePresentKHR(m_device->present_queue(), &present_info));

	m_frame_number++;
	m_current_frame = (m_current_frame + 1) % MAX_SYNC_FRAME;
}

void vk_context::begin_frame()
{
	VkFence fence = m_sync->get_inflight_fence(m_current_frame);
	vkWaitForFences(device(), 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device(), 1, &fence);

	// --- draw_2: Swapchain 이미지 요청
	uint32_t image_index;
	VK(vkAcquireNextImageKHR(
	    m_device->handle(),
	    m_swapchain->handle(),
	    UINT64_MAX,
	    m_sync->get_image_available_semaphore(m_current_frame),
	    VK_NULL_HANDLE,
	    &image_index));

	m_swapchain_image_frame = image_index;

	// --- draw_3: Command Buffer reset & begin
	VkCommandBuffer cmd = frames[m_current_frame].m_cmd;
	vkResetCommandBuffer(cmd, 0);

	VkCommandBufferBeginInfo begin_info = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK(vkBeginCommandBuffer(cmd, &begin_info));

	VK(vkEndCommandBuffer(cmd));
}

void vk_context::end_frame()
{
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore          wait_semaphore   = m_sync->get_image_available_semaphore(m_current_frame);
	VkSemaphore          signal_semaphore = m_sync->get_render_finished_semaphore(m_current_frame);
	VkPipelineStageFlags wait_stage       = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit_info.waitSemaphoreCount   = 1;
	submit_info.pWaitSemaphores      = &wait_semaphore;
	submit_info.pWaitDstStageMask    = &wait_stage;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &frames[m_current_frame].m_cmd;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &signal_semaphore;

	VkFence fence = m_sync->get_inflight_fence(m_current_frame);
	VK(vkQueueSubmit(m_device->graphics_queue(), 1, &submit_info, fence));

	// --- draw_6: Present
	VkPresentInfoKHR present_info{};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &signal_semaphore;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = m_swapchain->handle_ptr();
	present_info.pImageIndices      = &m_swapchain_image_frame;

	VK(vkQueuePresentKHR(m_device->present_queue(), &present_info));

	m_frame_number++;
	m_current_frame = (m_current_frame + 1) % MAX_SYNC_FRAME;
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

VkDevice vk_context::device() const
{
	return m_device->handle();
}

VkSwapchainKHR vk_context::swapchain() const
{
	return m_swapchain->handle();
}

void vk_context::create_command_objects()
{
	VkDevice device                = m_device->handle();
	uint32_t graphics_queue_family = m_device->graphics_queue_family_index();

	for(uint32_t i = 0; i < MAX_SYNC_FRAME; i++) {
		// Command Pool 생성
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = graphics_queue_family;

		VK(vkCreateCommandPool(device, &pool_info, nullptr, &frames[i].m_cmd_pool));

		// Command Buffer 할당
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool        = frames[i].m_cmd_pool;
		alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		VK(vkAllocateCommandBuffers(device, &alloc_info, &frames[i].m_cmd));
	}
}

void vk_context::destroy_command_objects()
{
	VkDevice device = m_device->handle();

	for(uint32_t i = 0; i < MAX_SYNC_FRAME; i++) {
		if(frames[i].m_cmd != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(device, frames[i].m_cmd_pool, 1, &frames[i].m_cmd);
			frames[i].m_cmd = VK_NULL_HANDLE;
		}

		if(frames[i].m_cmd_pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, frames[i].m_cmd_pool, nullptr);
			frames[i].m_cmd_pool = VK_NULL_HANDLE;
		}
	}
}

VkCommandBufferBeginInfo vk_context::command_buffer_begin_info(VkCommandBufferUsageFlags flags)
{
	VkCommandBufferBeginInfo info{};
	info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext            = nullptr;
	info.flags            = flags;        // 한 번만 사용할 CommandBuffer라면
	info.pInheritanceInfo = nullptr;        // secondary command buffer가 아니므로 nullptr
	return info;
}

void vk_context::transition_image(VkCommandBuffer cmd, VkImage image,
                                  VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext               = nullptr;
	barrier.oldLayout           = oldLayout;
	barrier.newLayout           = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image               = image;

	// subresource range 설정
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;

	if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// 접근 마스크 (Access Masks)
	VkPipelineStageFlags srcStage = 0;
	VkPipelineStageFlags dstStage = 0;

	switch(oldLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			srcStage              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		default:
			barrier.srcAccessMask = 0;
			srcStage              = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
	}

	switch(newLayout) {
		case VK_IMAGE_LAYOUT_GENERAL:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			dstStage              = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dstStage              = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		default:
			barrier.dstAccessMask = 0;
			dstStage              = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;
	}

	vkCmdPipelineBarrier(
	    cmd,
	    srcStage,
	    dstStage,
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &barrier);
}

}        // namespace juce
