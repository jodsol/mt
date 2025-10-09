#include "vk_swapchain.h"
#include <algorithm>
#include "vk_logical_device.h"
#include "vk_render_target.h"

namespace juce
{

vk_swapchain::vk_swapchain(const vk_logical_device* device, uint32_t cx, uint32_t cy) :
    m_physical_device(device->get_gpu()->handle),
    m_device(device->handle()),
    m_surface(device->surface())
{
	create_swapchain(cx, cy);

	int a = 0;
}

vk_swapchain::~vk_swapchain()
{
	destroy_swapchain();
}

bool vk_swapchain::create_swapchain(uint32_t width, uint32_t height)
{
	// 1. 스왑체인 지원 정보 가져오기
	SwapChainSupportDetails swapchain_support =
	    query_swapchain_support(m_physical_device, m_surface);

	// 2. format, present, extent 가져오기
	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
	VkPresentModeKHR   present_mode   = choose_swap_present_mode(swapchain_support.presentModes);
	VkExtent2D         extent         = choose_swap_extent(swapchain_support.capabilities, width, height);

	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
	if(swapchain_support.capabilities.maxImageCount > 0 &&
	   image_count > swapchain_support.capabilities.maxImageCount) {
		image_count = swapchain_support.capabilities.maxImageCount;
	}

	// 3. Swapchain 생성 정보
	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface          = m_surface;
	create_info.minImageCount    = image_count;
	create_info.imageFormat      = surface_format.format;
	create_info.imageColorSpace  = surface_format.colorSpace;
	create_info.imageExtent      = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	// 4. Queue family 설정 필요 (graphicsFamily, presentFamily)
	uint32_t queue_family_indices[] = {0, 0};
	if(queue_family_indices[0] != queue_family_indices[1]) {
		create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices   = queue_family_indices;
	}
	else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform   = swapchain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode    = present_mode;
	create_info.clipped        = VK_TRUE;
	create_info.oldSwapchain   = VK_NULL_HANDLE;

	// 5. 스왑체인 생성
	VK(vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_handle));

	// 6. 이미지 가져오기
	std::vector<VkImage> images;

	vkGetSwapchainImagesKHR(m_device, m_handle, &image_count, nullptr);
	images.resize(image_count);
	vkGetSwapchainImagesKHR(m_device, m_handle, &image_count, images.data());

	m_swapchain_image_format = surface_format.format;
	m_swapchain_extent       = extent;

	m_render_targets.resize(image_count);

	for(uint32_t i = 0; i < image_count; i++) {
		VkImage           image         = images[i];
		VkImageView       view          = create_image_view(images[i], m_swapchain_image_format);
		vk_render_target* render_target = debug_new vk_render_target();

		render_target->type   = render_target_type::color;
		render_target->device = m_device;
		render_target->image  = image;
		render_target->view   = view;
		render_target->mem    = nullptr;
		render_target->format = m_swapchain_image_format;
		render_target->extend = extent;
		render_target->layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

		render_target->clear_value.color = {0.f, 0.f, 0.f, 1.f};

		render_target->load_op  = load_operator::clear;
		render_target->store_op = store_operator::store;
		render_target->scope    = alloc_scope::imported;

		m_render_targets[i] = render_target;
	}

	return true;
}

void vk_swapchain::recreate_swapchain(uint32_t cx, uint32_t cy)
{
	if(cx == 0 || cy == 0) {
		return;
	}

	vkDeviceWaitIdle(m_device);

	destroy_swapchain();

	create_swapchain(cx, cy);
}

VkImage vk_swapchain::get_image(uint32_t index)
{
	return m_render_targets[index]->image;
}

VkImageView vk_swapchain::get_image_view(uint32_t index)
{
	return m_render_targets[index]->view;
}

vk_render_target* vk_swapchain::get_render_target(uint32_t index)
{
	return m_render_targets[index];
}

// vk_render_target* vk_swapchain::get_render_target(uint32_t index)
// {
// 	return m_render_targets[index];
// }

VkImageView vk_swapchain::create_image_view(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo view_info{};
	view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image                           = image;
	view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format                          = format;
	view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel   = 0;
	view_info.subresourceRange.levelCount     = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount     = 1;

	VkImageView image_view;
	VK(vkCreateImageView(m_device, &view_info, nullptr, &image_view));

	return image_view;
}

SwapChainSupportDetails vk_swapchain::query_swapchain_support(VkPhysicalDevice physical_device,
                                                              VkSurfaceKHR     surface)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

	if(format_count != 0) {
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count,
		                                     details.formats.data());
	}

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
	                                          nullptr);

	if(present_mode_count != 0) {
		details.presentModes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
		                                          details.presentModes.data());
	}

	return details;
}

bool vk_swapchain::destroy_swapchain()
{
	for(uint32_t i = 0; i < m_render_targets.size(); i++) {
		safe_delete(m_render_targets[i]);
	}
	m_render_targets.clear();
	// ImageView 제거
	if(m_handle != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(m_device, m_handle, nullptr);
		m_handle = VK_NULL_HANDLE;
	}

	return true;
}

VkSurfaceFormatKHR vk_swapchain::choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for(const auto& available_format : available_formats) {
		if(available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
		   available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_format;
		}
	}

	return available_formats[0];
}

VkPresentModeKHR vk_swapchain::choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes)
{
	for(const auto& available_present_mode : available_present_modes) {
		if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return available_present_mode;
		}
		// if(available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		// 	return available_present_mode;
		// }
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vk_swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities,
                                            uint32_t cur_width, uint32_t cur_height)
{
	VkExtent2D extent;
	if(capabilities.currentExtent.width != UINT32_MAX) {
		extent = capabilities.currentExtent;
	}
	else {
		extent.width  = std::clamp(cur_width, capabilities.minImageExtent.width,
		                           capabilities.maxImageExtent.width);
		extent.height = std::clamp(cur_height, capabilities.minImageExtent.height,
		                           capabilities.maxImageExtent.height);
	}

	return extent;
}

}        // namespace juce
