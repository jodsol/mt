#include "vk_context.h"
#include "logger.h"
#include <iostream>
#include <set>
#include <algorithm>

namespace juce {

VkContext::VkContext() 
    : m_instance(VK_NULL_HANDLE)
    , m_physical_device(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE)
    , m_graphics_queue(VK_NULL_HANDLE)
    , m_present_queue(VK_NULL_HANDLE)
    , m_surface(VK_NULL_HANDLE)
    , m_graphics_queue_family(UINT32_MAX)
    , m_present_queue_family(UINT32_MAX)
{
}

VkContext::~VkContext() {
    cleanup();
}

bool VkContext::initialize(HWND hwnd, HINSTANCE hinstance) {
    m_hwnd = hwnd;
    m_hinstance = hinstance;

    if (!create_instance()) {
        log_error("Failed to create Vulkan instance");
        return false;
    }

    if (!create_surface()) {
        log_error("Failed to create Vulkan surface");
        return false;
    }

    if (!pick_physical_device()) {
        log_error("Failed to pick physical device");
        return false;
    }

    if (!create_logical_device()) {
        log_error("Failed to create logical device");
        return false;
    }

    if (!create_command_pool()) {
        log_error("Failed to create command pool");
        return false;
    }

    log_info("Vulkan context initialized successfully");
    return true;
}

void VkContext::cleanup() {
    if (m_command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
        m_command_pool = VK_NULL_HANDLE;
    }

    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

bool VkContext::create_instance() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Juce Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Juce Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        log_error("Failed to create Vulkan instance. Error code: %d", result);
        return false;
    }

    return true;
}

bool VkContext::create_surface() {
    VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = m_hwnd;
    create_info.hinstance = m_hinstance;

    VkResult result = vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
        log_error("Failed to create Win32 surface. Error code: %d", result);
        return false;
    }

    return true;
}

bool VkContext::pick_physical_device() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

    if (device_count == 0) {
        log_error("No Vulkan capable devices found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

    for (const auto& device : devices) {
        if (is_device_suitable(device)) {
            m_physical_device = device;
            break;
        }
    }

    if (m_physical_device == VK_NULL_HANDLE) {
        log_error("No suitable physical device found");
        return false;
    }

    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(m_physical_device, &device_props);
    log_info("Selected GPU: %s", device_props.deviceName);

    return true;
}

bool VkContext::is_device_suitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = find_queue_families(device);
    
    bool extensions_supported = check_device_extension_support(device);
    
    bool swapchain_adequate = false;
    if (extensions_supported) {
        SwapchainSupportDetails swapchain_support = query_swapchain_support(device);
        swapchain_adequate = !swapchain_support.formats.empty() && 
                           !swapchain_support.present_modes.empty();
    }

    return indices.is_complete() && extensions_supported && swapchain_adequate;
}

VkContext::QueueFamilyIndices VkContext::find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);
        if (present_support) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool VkContext::check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(m_device_extensions.begin(), m_device_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

VkContext::SwapchainSupportDetails VkContext::query_swapchain_support(VkPhysicalDevice device) const {
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

bool VkContext::create_logical_device() {
    QueueFamilyIndices indices = find_queue_families(m_physical_device);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(m_device_extensions.size());
    create_info.ppEnabledExtensionNames = m_device_extensions.data();
    create_info.enabledLayerCount = 0;

    VkResult result = vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        log_error("Failed to create logical device. Error code: %d", result);
        return false;
    }

    vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);

    m_graphics_queue_family = indices.graphics_family.value();
    m_present_queue_family = indices.present_family.value();

    return true;
}

bool VkContext::create_command_pool() {
    QueueFamilyIndices indices = find_queue_families(m_physical_device);

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // 필요하면 커맨드 버퍼 재사용 가능
    pool_info.queueFamilyIndex = indices.graphics_family.value();

    VkResult result = vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool);
    if (result != VK_SUCCESS) {
        log_error("Failed to create command pool. Error code: %d", result);
        return false;
    }

    return true;
}


VkDevice VkContext::get_device() const {
    return m_device;
}

VkPhysicalDevice VkContext::get_physical_device() const {
    return m_physical_device;
}

VkQueue VkContext::get_graphics_queue() const {
    return m_graphics_queue;
}

VkQueue VkContext::get_present_queue() const {
    return m_present_queue;
}

VkSurfaceKHR VkContext::get_surface() const {
    return m_surface;
}

uint32_t VkContext::get_graphics_queue_family() const {
    return m_graphics_queue_family;
}

uint32_t VkContext::get_present_queue_family() const {
    return m_present_queue_family;
}

VkContext::SwapchainSupportDetails VkContext::get_swapchain_support() const {
    return query_swapchain_support(m_physical_device);
}

} // namespace juce