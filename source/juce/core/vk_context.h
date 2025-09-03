#pragma once

#include "win32_config.h"
#include <vector>
#include <optional>
#include <string>

namespace juce
{

class vk_context
{
public:
    // --- swapchain 지원 구조체 ---
    struct swapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    // --- 큐 패밀리 인덱스 ---
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete() const
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    // --- 생성자 & 소멸자 ---
    vk_context();
    ~vk_context();

    // --- 초기화 & 정리 ---
    bool initialize(HWND hwnd, HINSTANCE hinstance);
    void cleanup();

    // --- Getters ---
    VkDevice get_device() const;
    VkPhysicalDevice get_physical_device() const;
    VkQueue get_graphics_queue() const;
    VkQueue get_present_queue() const;
    VkSurfaceKHR get_surface() const;
    VkCommandPool get_command_pool() const;
    uint32_t get_graphics_queue_family() const;
    uint32_t get_present_queue_family() const;
    swapchainSupportDetails get_swapchain_support() const;

private:
    // --- 내부 초기화 단계 ---
    bool create_instance();
    void setup_debug_messenger();
    bool create_surface();
    bool pick_physical_device();
    bool create_logical_device();
    bool create_command_pool();

    // --- 헬퍼 함수 ---
    bool check_validation_layer_support();
    std::vector<const char*> get_required_extensions();
    bool is_device_suitable(VkPhysicalDevice device);
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);
    swapchainSupportDetails query_swapchain_support(VkPhysicalDevice device) const;

    // --- Vulkan 객체 ---
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    VkSurfaceKHR m_surface;
    VkCommandPool m_command_pool;
    VkDebugUtilsMessengerEXT m_debug_messenger;

    // --- 큐 패밀리 인덱스 ---
    uint32_t m_graphics_queue_family;
    uint32_t m_present_queue_family;

    // --- Win32 핸들 ---
    HWND m_hwnd;
    HINSTANCE m_hinstance;

    // --- 설정값 ---
    const std::vector<const char*> m_validation_layers = {
        "VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_device_extensions = {
        VK_KHR_swapchain_EXTENSION_NAME};
    const bool m_enable_validation_layers =
#ifdef NDEBUG
        false;
#else
        true;
#endif
};

} // namespace juce
