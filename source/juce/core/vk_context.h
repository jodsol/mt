#pragma once
#include "win32_config.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace juce {

class VkContext {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete() {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

public:
    VkContext();
    ~VkContext();

    bool initialize(HWND hwnd, HINSTANCE hinstance);
    void cleanup();

    VkDevice get_device() const;
    VkPhysicalDevice get_physical_device() const;
    VkQueue get_graphics_queue() const;
    VkQueue get_present_queue() const;
    VkSurfaceKHR get_surface() const;
    uint32_t get_graphics_queue_family() const;
    uint32_t get_present_queue_family() const;
    SwapchainSupportDetails get_swapchain_support() const;

private:
    bool create_instance();
    bool create_surface();
    bool pick_physical_device();
    bool create_logical_device();
    bool create_command_pool();

    VkCommandPool get_command_pool() const;
    bool is_device_suitable(VkPhysicalDevice device);
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);
    SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device) const;

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    VkSurfaceKHR m_surface;
    VkCommandPool m_command_pool;
    
    HWND m_hwnd;
    HINSTANCE m_hinstance;
    
    uint32_t m_graphics_queue_family;
    uint32_t m_present_queue_family;

    const std::vector<const char*> m_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
};

} // namespace juce