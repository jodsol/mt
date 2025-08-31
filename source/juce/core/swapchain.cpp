#include "win32_config.h"
#include "swapchain.h"
#include "logger.h"
#include <algorithm>
#include <limits>
#include <cstdint>
#include <stdexcept>

namespace juce {

Swapchain::Swapchain()
    : m_swapchain(VK_NULL_HANDLE)
    , m_format{}
    , m_extent{}
    , m_depth_image(VK_NULL_HANDLE)
    , m_depth_image_memory(VK_NULL_HANDLE)
    , m_depth_image_view(VK_NULL_HANDLE)
    , m_context(nullptr)
    , m_width(0)
    , m_height(0)
{
}

Swapchain::~Swapchain() {
    cleanup();
}

bool Swapchain::initialize(VkContext* context, uint32_t width, uint32_t height) {
    if (!context) {
        log_error("Invalid context provided to Swapchain::initialize");
        return false;
    }

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

    if (!create_depth_resources()) {
        log_error("Failed to create depth resources");
        return false;
    }

    /**
    if (!create_framebuffers(renderer->get_render_pass())) {
        log_error("Failed to create framebuffers");
        return false;
    }
    */

    log_info("Swapchain initialized successfully");
    return true;
}

void Swapchain::cleanup() {
    if (!m_context || m_context->get_device() == VK_NULL_HANDLE)
        return;

    VkDevice device = m_context->get_device();

    // Wait for device to be idle before cleanup
    vkDeviceWaitIdle(device);

    // 1. Framebuffers 삭제
    for (auto fb : m_framebuffers) {
        if (fb != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, fb, nullptr);
        }
    }
    m_framebuffers.clear();

    // 2. Depth 이미지 관련 리소스 삭제
    if (m_depth_image_view != VK_NULL_HANDLE) {
        vkDestroyImageView(device, m_depth_image_view, nullptr);
        m_depth_image_view = VK_NULL_HANDLE;
    }
    if (m_depth_image != VK_NULL_HANDLE) {
        vkDestroyImage(device, m_depth_image, nullptr);
        m_depth_image = VK_NULL_HANDLE;
    }
    if (m_depth_image_memory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_depth_image_memory, nullptr);
        m_depth_image_memory = VK_NULL_HANDLE;
    }

    // 3. Swapchain 이미지 뷰 삭제
    for (auto image_view : m_image_views) {
        if (image_view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, image_view, nullptr);
        }
    }
    m_image_views.clear();
    m_images.clear(); // 이미지 핸들들도 정리

    // 4. Swapchain 자체 삭제
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

bool Swapchain::recreate(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    // Wait for device to be idle before recreation
    if (m_context && m_context->get_device() != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_context->get_device());
    }
    
    cleanup();
    
    if (!create_swapchain()) {
        log_error("Failed to recreate swapchain");
        return false;
    }

    if (!create_image_views()) {
        log_error("Failed to recreate image views");
        return false;
    }

    if (!create_depth_resources()) {
        log_error("Failed to recreate depth resources");
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

bool Swapchain::create_framebuffers(VkRenderPass renderPass) {
    if (renderPass == VK_NULL_HANDLE) {
        log_error("Invalid render pass provided to create_framebuffers");
        return false;
    }

    m_framebuffers.resize(m_image_views.size());
    
    for (size_t i = 0; i < m_image_views.size(); i++) {
        std::vector<VkImageView> attachments;
        attachments.push_back(m_image_views[i]);
        
        // Add depth attachment if available
        if (m_depth_image_view != VK_NULL_HANDLE) {
            attachments.push_back(m_depth_image_view);
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(m_context->get_device(), &framebufferInfo, nullptr, &m_framebuffers[i]);
        if (result != VK_SUCCESS) {
            log_error("Failed to create framebuffer %zu. Error code: %d", i, result);
            return false;
        }
    }
    return true;
}

bool Swapchain::create_depth_resources() {
    try {
        VkFormat depthFormat = find_depth_format();

        // 깊이 이미지 생성
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_extent.width;
        imageInfo.extent.height = m_extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateImage(m_context->get_device(), &imageInfo, nullptr, &m_depth_image);
        if (result != VK_SUCCESS) {
            log_error("Failed to create depth image. Error code: %d", result);
            return false;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_context->get_device(), m_depth_image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        
        // find_memory_type을 직접 구현하거나 VkContext에서 다른 메서드명을 사용
        uint32_t memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        result = vkAllocateMemory(m_context->get_device(), &allocInfo, nullptr, &m_depth_image_memory);
        if (result != VK_SUCCESS) {
            log_error("Failed to allocate depth image memory. Error code: %d", result);
            return false;
        }

        vkBindImageMemory(m_context->get_device(), m_depth_image, m_depth_image_memory, 0);

        // 이미지 뷰 생성
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_depth_image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(m_context->get_device(), &viewInfo, nullptr, &m_depth_image_view);
        if (result != VK_SUCCESS) {
            log_error("Failed to create depth image view. Error code: %d", result);
            return false;
        }

        return true;
    }
    catch (const std::exception& e) {
        log_error("Exception in create_depth_resources: %s", e.what());
        return false;
    }
}

VkSurfaceFormatKHR Swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    if (available_formats.empty()) {
        log_error("No surface formats available");
        // Return a default format, but this should be handled by caller
        return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && 
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR Swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    if (available_present_modes.empty()) {
        log_error("No present modes available");
        return VK_PRESENT_MODE_FIFO_KHR; // This should always be supported
    }

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

VkResult Swapchain::acquire_next_image(uint32_t* imageIndex, VkSemaphore semaphore) {
    if (!imageIndex) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return vkAcquireNextImageKHR(m_context->get_device(), m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, imageIndex);
}

VkResult Swapchain::present_image(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

VkFormat Swapchain::find_supported_format() {
    // 헤더에서 선언된 대로 매개변수 없는 버전으로 수정
    std::vector<VkFormat> candidates = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_context->get_physical_device(), format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    log_error("Failed to find supported format!");
    throw std::runtime_error("failed to find supported format!");
}

VkFormat Swapchain::find_depth_format() {
    return find_supported_format();
}

// Getter methods
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

const std::vector<VkFramebuffer>& Swapchain::get_framebuffers() const {
    return m_framebuffers;
}

// find_memory_type 헬퍼 함수 추가
uint32_t Swapchain::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_context->get_physical_device(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

// get_depth_image_view 메서드 제거 (헤더에 없음)

uint32_t Swapchain::get_image_count() const {
    return static_cast<uint32_t>(m_images.size());
}

} // namespace juce