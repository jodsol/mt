// physical_device -> logical_device -> swapchain
#pragma once

#include <juce/core/typedef.h>
#include "vk_handle.h"
#include "vk_config.h"
#include <vector>

namespace juce
{

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR        capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentModes;
};

class vk_swapchain : public vk_handle<VkSwapchainKHR>
{
public:
	vk_swapchain(const vk_device* device, uint32_t cx, uint32_t cy);
	~vk_swapchain();

	bool create_swapchain(uint32_t width, uint32_t height);
	bool destroy_swapchain();
	void recreate_swapchain(uint32_t cur_width, uint32_t cur_height);

	const std::vector<VkImage>& get_images() const { return m_swapchain_images; }

	VkImage                         get_image(uint32_t index) { return m_swapchain_images[index]; }
	const std::vector<VkImageView>& get_image_views() const { return m_swapchain_image_views; }

	VkImageView get_image_view(uint32_t index) const { return m_swapchain_image_views[index]; }
	VkExtent2D  extent() const { return m_swapchain_extent; }

private:
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice         m_device          = VK_NULL_HANDLE;
	VkSurfaceKHR     m_surface         = VK_NULL_HANDLE;

	VkFormat m_swapchain_image_format = VK_FORMAT_UNDEFINED;

	std::vector<VkImage>     m_swapchain_images;
	std::vector<VkImageView> m_swapchain_image_views;

	// vk_render_target

	VkExtent2D m_swapchain_extent = {0, 0};

	SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice physical_device,
	                                                VkSurfaceKHR     surface);
	VkSurfaceFormatKHR
	    choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR
	    choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
	VkExtent2D  choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width,
	                               uint32_t height);
	void        create_image_views();
	VkImageView create_image_view(VkImage image, VkFormat format);
};
}        // namespace juce
