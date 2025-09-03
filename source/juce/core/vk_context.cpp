#include "vk_context.h"
#include "win32_config.h"
#include "logger.h"

#include <iostream>
#include <set>
#include <algorithm>
#include <stdexcept>

// --- Debug Callback Function ---
// Called when a message is generated from a Vulkan validation layer.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    // Select log level based on severity.
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        log_error("Vulkan Validation Layer: %s", pCallbackData->pMessage);
    }
    else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        log_warn("Vulkan Validation Layer: %s", pCallbackData->pMessage);
    }
    else
    {
        // Informational messages are ignored for now.
    }

    return VK_FALSE; // Indicates the callback has been handled.
}

// Proxy function to create a debug messenger
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Proxy function to destroy a debug messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

namespace juce
{

// Helper function to populate the debug messenger create info
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

vk_context::vk_context()
    : m_instance(VK_NULL_HANDLE), m_physical_device(VK_NULL_HANDLE), m_device(VK_NULL_HANDLE), m_graphics_queue(VK_NULL_HANDLE), m_present_queue(VK_NULL_HANDLE), m_surface(VK_NULL_HANDLE), m_command_pool(VK_NULL_HANDLE), m_debug_messenger(VK_NULL_HANDLE), m_graphics_queue_family(UINT32_MAX), m_present_queue_family(UINT32_MAX), m_hwnd(nullptr), m_hinstance(nullptr)
{
}

vk_context::~vk_context()
{
    cleanup();
}

bool vk_context::initialize(HWND hwnd, HINSTANCE hinstance)
{
    m_hwnd = hwnd;
    m_hinstance = hinstance;

    try
    {
        if (!create_instance())
        {
            return false;
        }
        setup_debug_messenger();
        if (!create_surface())
        {
            return false;
        }
        if (!pick_physical_device())
        {
            return false;
        }
        if (!create_logical_device())
        {
            return false;
        }
        if (!create_command_pool())
        {
            return false;
        }
    }
    catch (const std::exception& e)
    {
        log_error("Vulkan context initialization failed: %s", e.what());
        // Clean up partially created resources
        cleanup();
        return false;
    }

    log_info("Vulkan context initialized successfully");
    return true;
}

void vk_context::cleanup()
{
    // Destroy device-dependent objects first
    if (m_command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
        m_command_pool = VK_NULL_HANDLE;
    }

    if (m_device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    // Destroy instance-dependent objects
    if (m_enable_validation_layers && m_debug_messenger != VK_NULL_HANDLE)
    {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        m_debug_messenger = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

bool vk_context::create_instance()
{
    if (m_enable_validation_layers && !check_validation_layer_support())
    {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkapplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_application_INFO;
    app_info.papplicationName = "Juce Engine";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Juce Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    auto extensions = get_required_extensions();

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.papplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};

    // Set up validation layers and the debug messenger pNext chain
    if (m_enable_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
        create_info.ppEnabledLayerNames = m_validation_layers.data();

        populateDebugMessengerCreateInfo(debug_create_info);
        create_info.pNext = &debug_create_info;
    }
    else
    {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
    {
        log_error("Failed to create Vulkan instance.");
        return false;
    }

    return true;
}

void vk_context::setup_debug_messenger()
{
    if (!m_enable_validation_layers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    populateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

bool vk_context::create_surface()
{
    VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = m_hwnd;
    create_info.hinstance = m_hinstance;

    if (vkCreateWin32SurfaceKHR(m_instance, &create_info, nullptr, &m_surface) != VK_SUCCESS)
    {
        log_error("Failed to create Win32 surface.");
        return false;
    }
    return true;
}

bool vk_context::pick_physical_device()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

    if (device_count == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

    for (const auto& device : devices)
    {
        if (is_device_suitable(device))
        {
            m_physical_device = device;
            break;
        }
    }

    if (m_physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(m_physical_device, &device_props);
    log_info("Selected GPU: %s", device_props.deviceName);

    return true;
}

bool vk_context::is_device_suitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = find_queue_families(device);
    bool extensions_supported = check_device_extension_support(device);

    bool swapchain_adequate = false;
    if (extensions_supported)
    {
        swapchainSupportDetails swapchain_support = query_swapchain_support(device);
        swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
    }

    return indices.is_complete() && extensions_supported && swapchain_adequate;
}

vk_context::QueueFamilyIndices vk_context::find_queue_families(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);
        if (present_support)
        {
            indices.present_family = i;
        }

        if (indices.is_complete())
        {
            break;
        }
        i++;
    }

    return indices;
}

bool vk_context::check_device_extension_support(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(m_device_extensions.begin(), m_device_extensions.end());

    for (const auto& extension : available_extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

vk_context::swapchainSupportDetails vk_context::query_swapchain_support(VkPhysicalDevice device) const
{
    swapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr);
    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, details.present_modes.data());
    }
    return details;
}

bool vk_context::create_logical_device()
{
    QueueFamilyIndices indices = find_queue_families(m_physical_device);
    if (!indices.is_complete())
    {
        throw std::runtime_error("Could not find all required queue families.");
    }

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {
        indices.graphics_family.value(),
        indices.present_family.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
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

    // For modern Vulkan, validation layers are set at the instance level.
    if (m_enable_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
        create_info.ppEnabledLayerNames = m_validation_layers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS)
    {
        log_error("Failed to create logical device.");
        return false;
    }

    vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);

    m_graphics_queue_family = indices.graphics_family.value();
    m_present_queue_family = indices.present_family.value();

    return true;
}

bool vk_context::create_command_pool()
{
    QueueFamilyIndices indices = find_queue_families(m_physical_device);
    if (!indices.graphics_family.has_value())
    {
        throw std::runtime_error("Graphics queue family not found for command pool creation.");
    }

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = indices.graphics_family.value();

    if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS)
    {
        log_error("Failed to create command pool.");
        return false;
    }

    return true;
}

// --- Helper Functions ---
bool vk_context::check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : m_validation_layers)
    {
        bool layer_found = false;
        for (const auto& layer_properties : available_layers)
        {
            if (strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
        {
            return false;
        }
    }
    return true;
}

std::vector<const char*> vk_context::get_required_extensions()
{
    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME};

    if (m_enable_validation_layers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}

// --- Getters ---
VkDevice vk_context::get_device() const { return m_device; }
VkPhysicalDevice vk_context::get_physical_device() const { return m_physical_device; }
VkQueue vk_context::get_graphics_queue() const { return m_graphics_queue; }
VkQueue vk_context::get_present_queue() const { return m_present_queue; }
VkSurfaceKHR vk_context::get_surface() const { return m_surface; }
VkCommandPool vk_context::get_command_pool() const { return m_command_pool; }
uint32_t vk_context::get_graphics_queue_family() const { return m_graphics_queue_family; }
uint32_t vk_context::get_present_queue_family() const { return m_present_queue_family; }
vk_context::swapchainSupportDetails vk_context::get_swapchain_support() const { return query_swapchain_support(m_physical_device); }

} // namespace juce
