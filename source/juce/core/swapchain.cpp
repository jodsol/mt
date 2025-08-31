#include "win32_config.h"
#include "swapchain.h"
#include "logger.h"
#include <algorithm>
#include <limits>
#include <cstdint>


namespace juce {

Swapchain::Swapchain()
    : m_swapchain(VK_NULL_HANDLE)
    , m_format{}
    , m_extent{}
{
}

Swapchain::~Swapchain() {
    cleanup();
}

bool Swapchain::initialize(VkContext* context, uint32_t width, uint32_t height) {
    m_context = context;
    m_width = width;
    m_height = height;

    if (!create_swapchain()) {
        log_error("Failed to create swapchain");
        return false;
    }

    if (!create_image_views()) {
        log_error("Failed to create image views");
        return false;
    }

    log_info("Swapchain initialized successfully");
    return true;
}

void Swapchain::cleanup() {
    if (m_context && m_context->get_device() != VK_NULL_HANDLE) {
        for (auto image_view : m_image_views) {
            vkDestroyImageView(m_context->get_device(), image_view, nullptr);
        }
        m_image_views.clear();

        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(m_context->get_device(), m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }
}

bool Swapchain::recreate(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    vkDeviceWaitIdle(m_context->get_device());
    
    cleanup();
    
    if (!create_swapchain()) {
        log_error("Failed to recreate swapchain");
        return false;
    }

    if (!create_image_views()) {
        log_error("Failed to recreate image views");
        return false;
    }

    log_info("Swapchain recreated successfully");
    return true;
}

bool Swapchain::create_swapchain() {
    VkContext::SwapchainSupportDetails swapchain_support = m_context->get_swapchain_support();

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && 
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_context->get_surface();
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = {
        m_context->get_graphics_queue_family(),
        m_context->get_present_queue_family()
    };

    if (m_context->get_graphics_queue_family() != m_context->get_present_queue_family()) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(m_context->get_device(), &create_info, nullptr, &m_swapchain);
    if (result != VK_SUCCESS) {
        log_error("Failed to create swapchain. Error code: %d", result);
        return false;
    }

    vkGetSwapchainImagesKHR(m_context->get_device(), m_swapchain, &image_count, nullptr);
    m_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_context->get_device(), m_swapchain, &image_count, m_images.data());

    m_format = surface_format.format;
    m_extent = extent;

    return true;
}

bool Swapchain::create_image_views() {
    m_image_views.resize(m_images.size());

    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = m_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = m_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(m_context->get_device(), &create_info, nullptr, &m_image_views[i]);
        if (result != VK_SUCCESS) {
            log_error("Failed to create image view %zu. Error code: %d", i, result);
            return false;
        }
    }

    return true;
}

VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && 
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR Swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actual_extent = { m_width, m_height };

        actual_extent.width = std::clamp(actual_extent.width, 
            capabilities.minImageExtent.width, 
            capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, 
            capabilities.minImageExtent.height, 
            capabilities.maxImageExtent.height);

        return actual_extent;
    }
}


VkSwapchainKHR Swapchain::get_swapchain() const {
    return m_swapchain;
}

VkFormat Swapchain::get_format() const {
    return m_format;
}

VkExtent2D Swapchain::get_extent() const {
    return m_extent;
}

const std::vector<VkImage>& Swapchain::get_images() const {
    return m_images;
}

const std::vector<VkImageView>& Swapchain::get_image_views() const {
    return m_image_views;
}

uint32_t Swapchain::get_image_count() const {
    return static_cast<uint32_t>(m_images.size());
}

} // namespace juce