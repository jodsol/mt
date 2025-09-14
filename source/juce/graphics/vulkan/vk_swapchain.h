#pragma once

#include <juce/core/typedef.h>
#include "vk_handle.h"
#include "vk_config.h"

namespace juce
{

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class vk_swapchain: vk_handle<VkSwapchainKHR> 
{
public:
	vk_swapchain();
	~vk_swapchain();
     
	bool create_swapchain(uint32_t width, uint32_t height);
	bool destroy_swapchain();
    void recreate_swapchain();


private:
	VkFormat m_swapchain_image_format = VK_NULL_HANDLE;

	std::vector<VkImage> m_swapchain_images;
	std::vector<VkImageView> m_swapchain_image_views;
	VkExtent2D m_swapchain_extent = VK_NULL_HANDLE;

    SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device);
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
    void create_image_views();
    VkImageView create_image_view(VkImage image, VkFormat format);

}
} // namespace juce
