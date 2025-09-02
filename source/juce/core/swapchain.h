#pragma once

#include "win32_config.h"
#include <vector>
#include <cstdint>

namespace juce {

class VKContext;

/**
 * Swapchain wrapper class
 * - 관리: Swapchain, ImageViews, DepthBuffer, Framebuffers
 * - 기능: 생성/정리/재생성, 이미지 획득, 프레젠트
 */
class Swapchain {
public:
    Swapchain();
    ~Swapchain();

    // Swapchain 및 관련 리소스 초기화
    bool initialize(VKContext* context, uint32_t width, uint32_t height);

    // 리소스 정리
    void cleanup();

    // 창 크기 변경 시 swapchain 재생성
    bool recreate(uint32_t width, uint32_t height);

    // RenderPass에 맞는 Framebuffer 생성
    bool create_framebuffers(VkRenderPass renderPass);
    
    void cleanup_framebuffers();

    // 이미지 획득
    VkResult acquire_next_image(uint32_t* imageIndex, VkSemaphore semaphore);

    // 프레젠트 큐 제출
    VkResult present_image(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore waitSemaphore);

    VkSwapchainKHR get_handle() const;
    VkFormat get_image_format() const;
    VkExtent2D get_extent() const;
    VkImageView get_image_view(uint32_t index) const;
    VkFramebuffer get_framebuffer(uint32_t index) const;
    uint32_t get_image_count() const;

private:
    // Swapchain 관련 생성
    bool create_swapchain();
    bool create_image_views();
    bool create_depth_resources();

    // 선택 헬퍼
    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

    // Depth 포맷/메모리
    VkFormat find_supported_format(const std::vector<VkFormat>& candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features);
    VkFormat find_depth_format();
    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
    VkSwapchainKHR m_swapchain;
    VkFormat m_format;
    VkExtent2D m_extent;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;
    std::vector<VkFramebuffer> m_framebuffers;

    VkImage m_depth_image;
    VkDeviceMemory m_depth_image_memory;
    VkImageView m_depth_image_view;

    VKContext* m_context; // 소유하지 않음
    uint32_t m_width;
    uint32_t m_height;
};

} // namespace juce
