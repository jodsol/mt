#include "vk_swapchain.h"
#include <stdexcept>
#include <algorithm>
#include <windows.h>

namespace juce
{

vk_swapchain::vk_swapchain()
{
}

vk_swapchain::~vk_swapchain()
{
    destroy_swapchain();
}

bool vk_swapchain::create_swapchain(VkPhysicalDevice physical_device, VkDevice device, uint32_t width, uint32_t height)
{
    // TODO: 실제로는 device, surface, physicalDevice 같은 핸들이 필요함
    // vk_handle<VkSwapchainKHR> 안에 device 관련 참조가 있다고 가정

    // 1. 스왑체인 지원 정보 가져오기
    SwapChainSupportDetails swapchain_support = query_swapchain_support(physical_device);
    
    // → getSwapChainSupport(physicalDevice, surface) 같은 헬퍼 함수가 필요함


    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.presentModes);
    VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    // 5. Swapchain 생성 정보
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = /* surface handle 필요 */ VK_NULL_HANDLE;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue family 설정 필요 (graphicsFamily, presentFamily)
    uint32_t queue_family_indices[] = {0, 0};
    if (queue_family_indices[0] != queue_family_indices[1]) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = swapChainSupport.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = presentMode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    // 6. 스왑체인 생성
    VkResult result = vkCreateSwapchainKHR(device,
                                           &createInfo,
                                           nullptr,
                                           &m_handle);
    if (result != VK_SUCCESS) {
        alert("Failed to create swapchain!");
        return false;
    }

    // 7. 이미지 가져오기
    vkGetSwapchainImagesKHR(device, m_handle, &imageCount, nullptr);
    m_swapchain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_handle, &imageCount, _swapchainImages.data());

    m_swapchain_image_format = surface_format.format;
    m_swapchain_extent = extent;

    // 8. ImageView 생성
    _swapchainImageViews.resize(_swapchainImages.size());
    for (size_t i = 0; i < _swapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = _swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = _swapchainImageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(/* device */ VK_NULL_HANDLE, &viewInfo, nullptr, &_swapchainImageViews[i]) != VK_SUCCESS) {
            LOG_ERROR("Failed to create image views for swapchain!");
            return false;
        }
    }

    return true;
}

void vk_swapchain::recreate_swapchain() {
}

void vk_swapchain::create_image_views() {
    m_swapchain_image_views.resize(m_swapchain_images.size());

    for (size_t i = 0; i < m_swapchain_images.size(); i++) {
        m_swapchain_image_views[i] = createImageView(m_swapchain_images[i], m_swapchain_image_format);
    }
}

VkImageView create_image_view(VkImage image, VkFormat format) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &view_info, nullptr, &imageView) != VK_SUCCESS) {
        alert("failed to create texture image view!");
        return false;
    }

    return imageView;
}


SwapChainSupportDetails vk_swapchain::query_swapchain_support(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.presentModes.data());
    }

    return details;
}


bool vk_swapchain::destroy_swapchain()
{
    // ImageView 제거
    for (auto imageView : _swapchainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(/* device */ VK_NULL_HANDLE, imageView, nullptr);
        }
    }
    _swapchainImageViews.clear();
    _swapchainImages.clear();

    if (m_handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(/* device */ VK_NULL_HANDLE, m_handle, nullptr);
        m_handle = VK_NULL_HANDLE;
    }

    return true;
}

VkSurfaceFormatKHR vk_swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR vk_swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes)
{
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

}

} // namespace juce
