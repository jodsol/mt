#include "vk_device.h"

namespace juce
{
vk_device::vk_device(
    VkInstance                      instance,
    VkSurfaceKHR                    surface,
    const std::vector<const char*>& extension)
{
	m_gpu.m_physical_device = choose_physical_device(instance, surface, extension);
}

vk_device::~vk_device()
{
	if (m_handle)
		vkDestroyDevice(m_handle, nullptr);
}

VkPhysicalDevice vk_device::choose_physical_device(
    VkInstance                      instance,
    VkSurfaceKHR                    surface,
    const std::vector<const char*>& extension)
{
	uint32_t device_num = 0;
	vkEnumeratePhysicalDevices(instance, &device_num, nullptr);

	std::vector<VkPhysicalDevice> physical_devices(device_num);
	vkEnumeratePhysicalDevices(instance, &device_num, physical_devices.data());
	log_debug("[juce] available physical devices count : %d", device_num);
	for (const auto& device : physical_devices) {
		// log_info("[juce] physical device name : %s", device);
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_properties.data());

		for (uint32_t i = 0; i < queue_family_count; ++i) {
			VkBool32 present_support = false;
			VkBool32 is_discrete     = false;

			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
			VkPhysicalDeviceProperties2 props{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

			vkGetPhysicalDeviceProperties2(device, &props);

			if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
				log_debug("[juce] found a suitable GPU : %s", props.properties.deviceName);
				m_gpu.graphics_quque_index = i;
				m_gpu.graphics_index       = i;
				break;
			}
		}
		if (m_gpu.graphics_index.has_value()) {
		}
		if (m_gpu.graphics_quque_index >= 0) {
			m_gpu.m_physical_device = device;
			break;
		}

		VkPhysicalDevice out = m_gpu;
	}

	// Vulkan 1.3 기능 지원
	VkPhysicalDeviceVulkan13Features feat13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
	VkPhysicalDeviceVulkan12Features feat12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
	VkPhysicalDeviceFeatures2        featrues{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	featrues.pNext = &feat12;
	feat12.pNext   = &feat13;
	feat13.pNext   = nullptr;

	return VkPhysicalDevice();

	// vertex_buffer* buffer;

	// info
	//     vk::....createbuffer(device ,&info, buffer * *pp_buffer);
	// {
	// }
}

}        // namespace juce
