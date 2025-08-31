#pragma once
#include "vk_context.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace juce {

class Swapchain {
public:
    Swapchain();
    ~Swapchain();

    bool initialize(VkContext* context, uint32_t width, uint32_t height);
    void cleanup();
    bool recreate(uint32_t width, uint32_t height);

    VkSwapchainKHR get_swapchain() const;
    VkFormat get_format() const;
    VkExtent2D get_extent() const;
    const std::vector<VkImage>& get_images() const;
    const std::vector<VkImageView>& get_image_views() const;
    uint32_t get_image_count() const;

private:
    bool create_swapchain();
    bool create_image_views();

    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

private:
    VkContext* m_context;
    VkSwapchainKHR m_swapchain;
    
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;
    
    VkFormat m_format;
    VkExtent2D m_extent;
    
    uint32_t m_width;
    uint32_t m_height;
};

} // namespace juce